#include "SpDatabase.h"
#include "Logger.h"
#include <QThread>
#include <QDir>
#include <QFile>
#include <QSqlRecord>

SpDatabase::SpDatabase(QObject *parent) :
    QObject(parent)
{
    DEBUG_LOG("Создан объект класса SpDatabase.");
    DEBUG_LOG("Список всех доступных драйверов базданных: " + QSqlDatabase().drivers().join(", "));
    QDir().mkdir("images");
}

void SpDatabase::setDatabasePath(QString path)
{
    mDatabasePath = path;
    DEBUG_LOG("[SpDatabase] Установлено имя базы данных: " + mDatabasePath);
}

bool SpDatabase::openDatabase()
{
    if (mDatabasePath.isEmpty()) {
        ERROR_LOG("[SpDatabase] Database path is empty.");
        return false;
    }

    if (QSqlDatabase::contains(QSqlDatabase::defaultConnection)) {
            mDatabase = QSqlDatabase::database();
    } else {
        mDatabase = QSqlDatabase::addDatabase("QSQLITE");
        mDatabase.setDatabaseName(mDatabasePath);
    }

    if (!mDatabase.open()) {
        ERROR_LOG("[SpDatabase] Could not open database.");
        return false;
    } else {
        if (!executeQuery("PRAGMA foreign_keys=on"))
            return false;

        if (!executeQuery("CREATE TABLE IF NOT EXISTS users ( "
                          "id                  INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
                          "login               TEXT, "
                          "password            TEXT, "
                          "email               TEXT, "
                          "name                TEXT, "
                          "avatar              TEXT, "
                          "created_at          TEXT, "
                          "updated_at          TEXT, "
                          "status              INT,"
                          "last_update         INT )")) {
            return false;
        }

        if (!executeQuery("CREATE TABLE IF NOT EXISTS messages ( "
                          "id                  INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
                          "sender_id           INT, "
                          "conversation_id     INT, "
                          "created_at          INT, "
                          "text_message        TEXT)")) {
            return false;
        }

        if (!executeQuery("CREATE TABLE IF NOT EXISTS images ( "
                          "id                  INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
                          "message_id          INT, "
                          "path_to_file        TEXT, "
                          "FOREIGN KEY (message_id) REFERENCES messages(id))")) {
            return false;
        }

        if (!executeQuery("CREATE TABLE IF NOT EXISTS relationship ( "
                          "user_one_id         INT, "
                          "user_two_id         INT, "
                          "status              INT, "
                          "action_user_id      INT, "
                          "PRIMARY KEY (user_one_id, user_two_id))")) {
            return false;
        }

        if (!executeQuery("CREATE TABLE IF NOT EXISTS conversation ( "
                          "id                  INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
                          "title               TEXT, "
                          "creator_id          INT, "
                          "created_at          TEXT, "
                          "updated_at          TEXT,"
                          "type                INT)")) {
            return false;
        }

        if (!executeQuery("CREATE TABLE IF NOT EXISTS participants ("
                          "conversation_id     INT, "
                          "user_id             INT, "
                          "type                INT, "
                          "FOREIGN KEY (conversation_id) REFERENCES conversation(id), "
                          "PRIMARY KEY (conversation_id, user_id))")) {
            return false;
        }

        if (!executeQuery("CREATE TABLE IF NOT EXISTS events ("
                          "target_user_id      INT, "
                          "type                INT, "
                          "PRIMARY KEY (target_user_id, type), "
                          "FOREIGN KEY (target_user_id) REFERENCES users(id))")) {
            return false;
        }

        DEBUG_LOG("[SpDatabase] Database successfully opened.");
        return true;
    }
}

void SpDatabase::clearDatabase()
{
    QList<QString> tableNames = {
        "participants",
        "conversation",
        "relationship",
        "messages",
        "users"
    };

    foreach (QString name, tableNames) {
        if (!executeQuery(QString("DELETE FROM %1").arg(name))) {
            return;
        }
    }
}

bool SpDatabase::verifyLogin(QString login) //CHECK
{
    if (login.length() < 3) {
        return false;
    }

    //Проверка на незанятость логина
    QueryValuesList list;
    if (!executeQuery(QString("SELECT * FROM users "
                              "WHERE login='%1'").arg(login), &list)) {
        return false;
    }
    bool isFree = list.isEmpty();
    return isFree;
}

bool SpDatabase::passwordCorrect(QString password)
{
    return password.length() > 2;
}

bool SpDatabase::setRelationshipStatus(int userOneId, int userTwoId, int status)
{
    int id_one = min(userOneId, userTwoId);
    int id_two = max(userOneId, userTwoId);

    QueryValuesList list;

    if (!executeQuery(QString("SELECT * FROM relationship "
                              "WHERE user_one_id = %1 "
                              "AND user_two_id = %2").arg(id_one).arg(id_two), &list)) {
        return false;
    }

    if (!list.isEmpty()) {
        if (!executeQuery(QString("UPDATE relationship SET status = %1, action_user_id = %2 "
                                  "WHERE user_one_id = %3 "
                                  "AND user_two_id = %4").arg(status).arg(userOneId).arg(id_one).arg(id_two))) {
            return false;
        }
    } else {
        if (!executeQuery(QString("INSERT INTO relationship (user_one_id,user_two_id,status,action_user_id) "
                                  "VALUES (%1,%2,%3,%4)").arg(id_one).arg(id_two).arg(status).arg(userOneId))) {
            return false;
        }
        createEvent(userOneId, RelationshipChanged);
        createEvent(userTwoId, RelationshipChanged);
    }

    return true;
}

int SpDatabase::createAccaunt(SpUser user, QString login, QString password)
{
    if (!verifyLogin(login)) {
        return SpPackage::FAIL_SIGN_UP_BAD_LOGIN;
    }

    if (!passwordCorrect(password)) {
        return SpPackage::FAIL_SIGN_UP_BAD_PASSWORD;
    }

    uint currentTime = QDateTime::currentDateTime().toTime_t();
    if (!executeQuery(QString("INSERT INTO users (login,password,email,name,created_at,updated_at,status,last_update) "
                              "VALUES ('%1','%2','%3','%4',%5,%6,%7,%8)")
                      .arg(login)
                      .arg(password)
                      .arg(user.mEmail)
                      .arg(user.mName)
                      .arg(currentTime)
                      .arg(currentTime)
                      .arg(SpUser::Offline)
                      .arg(currentTime))) {
        return SpPackage::ERR;
    }

    //Получаем id созданного пользователя
    QueryValuesList list;
    if (!executeQuery(QString("SELECT id FROM users "
                              "WHERE login='%1'").arg(login), &list)) {
        return SpPackage::ERR;
    }

    if (!list.isEmpty()) {
        int userId = list.at(0).at(0).toInt();
        QString pathToFile = PATTERN_AVATAR_PATH(userId);
        if (!saveImageToFile(user.mAvatar, pathToFile)) {
            return SpPackage::ERR;
        }
        if (!executeQuery(QString("UPDATE users SET avatar='%1' "
                                  "WHERE id=%2").arg(pathToFile).arg(userId))) {
            return SpPackage::ERR;
        }
        return SpPackage::SUCCESS_SIGN_UP;
    } else {
        ERROR_LOG("[SpDatabase] Запись о только что созданном пользователе не найдена.");
        return SpPackage::ERR;
    }
}

void SpDatabase::deleteAccaunt(SpUser user)
{
    if (!executeQuery(QString("UPDATE users SET status = %1 "
                              "WHERE id = %2").arg(SpUser::Deleted).arg(user.mId))) {
        return;
    }
}

void SpDatabase::changeUserStatus(int userId, int newStatus)
{
    if (!executeQuery(QString("UPDATE users SET status = %1 "
                              "WHERE id = %2").arg(newStatus).arg(userId))) {
        return;
    }
    foreach (auto friendId, getFriendsIdList(userId)) {
        createEvent(friendId, FriendStatusChanged);
    }
}

int SpDatabase::verifyAccount(SpUser *user, QString login, QString password)
{
    if (user == nullptr) {
        return SpPackage::ERR;
    }

    QueryValuesList list;
    if (!executeQuery(QString("SELECT * FROM users "
                              "WHERE login='%1' "
                              "AND password='%2'").arg(login).arg(password), &list)) {
        return SpPackage::ERR;
    }

    if (!list.isEmpty()) {
        auto row = list.at(0);
        user->mId = row.at(0).toInt();
        user->mName = row.at(4).toString();
        user->mEmail = row.at(3).toString();
        user->mAvatar.load(row.at(5).toString());
        user->mStatus = row.at(8).toInt();
        changeUserStatus(user->mId, SpUser::Online);
        return SpPackage::SUCCESS_SIGN_IN;;
    } else {
        return SpPackage::FAIL_SIGN_IN;
    }
}

SpConversation SpDatabase::createDialog(int userOneId, int userTwoId) //CHECK
{
    //Получаем список айди бесед первого пользователя
    QueryValuesList list;
    if (!executeQuery(QString("SELECT conversation_id "
                              "FROM participants "
                              "WHERE user_id = %1")
                      .arg(userOneId), &list)) {
        return SpConversation();
    }

    QList<int> converationsOne;
    foreach (auto row, list) {
        converationsOne.append(row.at(0).toInt());
    }

    //Получаем список айди бесед второго пользователя
    if (!executeQuery(QString("SELECT conversation_id "
                              "FROM participants "
                              "WHERE user_id = %1")
                      .arg(userOneId), &list)) {
        return SpConversation();
    }

    QList<int> converationsTwo;
    foreach (auto row, list) {
        converationsTwo.append(row.at(0).toInt());
    }

    foreach (auto idOne, converationsOne) {
        foreach (auto idTwo, converationsTwo) {
            if (idOne == idTwo) {
                SpConversation conv = getConversationById(idOne);
                if (conv.mType == SpConversation::Dialog) {
                    return conv;
                }
            }
        }
    }

    SpConversation dialog = createConversation(userOneId, "Dialog", SpConversation::Dialog);

    addParticipantToConversation(userOneId, SpUser::Creator, dialog.mId);
    addParticipantToConversation(userTwoId, SpUser::Invited, dialog.mId);

    return dialog;
}

SpConversation SpDatabase::createConversation(int creatorId, QString title, int type) //CHECK
{
    uint currentTime = QDateTime::currentDateTime().toTime_t();
    if (!executeQuery(QString("INSERT INTO conversation (title,creator_id,created_at,updated_at,type) "
                              "VALUES ('%1',%2,%3,%4,%5)")
                      .arg(title)
                      .arg(creatorId)
                      .arg(currentTime)
                      .arg(currentTime)
                      .arg(type))) {
        return SpConversation();
    }

    //TODO: Тут всего лишь надо получить айди новой беседы, для этого слишком много кода
    QueryValuesList list;
    if (!executeQuery(QString("SELECT * FROM conversation "
                              "WHERE created_at = %1 "
                              "AND creator_id = %2 "
                              "AND title = '%3'")
                      .arg(currentTime)
                      .arg(creatorId)
                      .arg(title), &list)) {
        return SpConversation();
    }

    if (!list.isEmpty()) {
        SpConversation newConversation;
        newConversation.mId = list.at(0).at(0).toInt();
        newConversation.mCreatorId = list.at(0).at(2).toInt();
        newConversation.mTitle = list.at(0).at(1).toString();
        newConversation.mCreatedAt = list.at(0).at(3).toDateTime();
        newConversation.mUpdatedAt = list.at(0).at(4).toDateTime();
        newConversation.mType = list.at(0).at(5).toInt();
        return newConversation;
    } else {
        return SpConversation();
    }
}

void SpDatabase::addParticipantToConversation(int userId, int userType, int conversationId)
{
    QueryValuesList list;
    if (!executeQuery(QString("SELECT * FROM participants "
                              "WHERE conversation_id=%1 "
                              "AND user_id=%2").arg(conversationId).arg(userId), &list)) {
        return;
    }
    if (!list.isEmpty()){
        return;
    }

    if (!executeQuery(QString("INSERT INTO participants (conversation_id,user_id,type) "
                              "VALUES (%1,%2,%3)").arg(conversationId).arg(userId).arg(userType))) {
        return;
    }

    createEvent(userId, ChangesInConversations);
}

void SpDatabase::addUserToFriends(int actionUserId, int anotherUserId)
{
    QPair<int,int> status = getRelationshipStatus(actionUserId, anotherUserId);
    bool conditionOne = status == UNKNOWN_USER;                                     //Пользователь1 предлагает дружбу пользователю2
    bool conditionTwo = status == QPair<int,int>(SpUser::Pending,anotherUserId);    //Пользователь1 соглашается дружить с пользователем2
    int id_one = min(actionUserId, anotherUserId);
    int id_two = max(actionUserId, anotherUserId);
    if (conditionOne) {        
        if (!executeQuery(QString("INSERT INTO relationship (user_one_id,user_two_id,status,action_user_id) "
                                  "VALUES (%1, %2, %3, %4)").arg(id_one).arg(id_two).arg(SpUser::Pending).arg(actionUserId))) {
            return;
        }
    }
    if (conditionTwo) {        
        if (!executeQuery(QString("UPDATE relationship SET status = %1, action_user_id = %2 "
                                  "WHERE user_one_id = %3 "
                                  "AND user_two_id = %4").arg(SpUser::Accepted).arg(actionUserId).arg(id_one).arg(id_two))) {
            return;
        }
        createDialog(actionUserId, anotherUserId);
    }

    createEvent(actionUserId, RelationshipChanged);
    createEvent(anotherUserId, RelationshipChanged);
}

void SpDatabase::removeUserFromFriends(int actionUserId, int anotherUserId)
{
    int id_one = min(actionUserId, anotherUserId);
    int id_two = max(actionUserId, anotherUserId);

    if (!executeQuery(QString("DELETE FROM relationship "
                              "WHERE user_one_id = %1 "
                              "AND user_two_id = %2").arg(id_one).arg(id_two))) {
        return;
    }

    createEvent(actionUserId, RelationshipChanged);
    createEvent(anotherUserId, RelationshipChanged);
}

QPair<int,int> SpDatabase::getRelationshipStatus(int userOneId, int userTwoId)
{
    int id_one = min(userOneId, userTwoId);
    int id_two = max(userOneId, userTwoId);

    QueryValuesList list;
    if (!executeQuery(QString("SELECT * FROM relationship "
                              "WHERE user_one_id = %1 "
                              "AND user_two_id = %2").arg(id_one).arg(id_two), &list)) {
        return INVALID_RELATIONSHIP;
    }

    if (!list.isEmpty()) {
        auto row = list.at(0);
        int status = row.at(2).toInt();
        int actionUserId = row.at(3).toInt();
        return QPair<int,int>(status, actionUserId);
    } else {
        return UNKNOWN_USER;
    }
}

bool SpDatabase::saveImageToFile(QImage image, QString path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        ERROR_LOG("[SpDatabase] Невозможно открыть файл [" + path + "].");
        return false;
    }
    file.close();

    if (!image.save(path)) {
        ERROR_LOG("[SpDatabase] Невозможно сохранить картинку в файл [" + path + "].");
        return false;
    }

    return true;
}

bool SpDatabase::executeQuery(QString query, QueryValuesList *result)
{
    QSqlQuery sqlQuery;
    sqlQuery.prepare(query);
    if (!sqlQuery.exec()) {
        LOG_FAILED_QUERY(sqlQuery);
        return false;
    } else {
        if (result) {
            result->clear();
            while (sqlQuery.next()) {
                QList<QVariant> row;
                QSqlRecord record = sqlQuery.record();
                for (int i = 0; i < record.count(); i++) {
                    row.append(record.value(i));
                }
                result->append(row);
            }
        }
        return true;
    }
}

void SpDatabase::blockUser(int actionUserId, int anotherUserId)
{
    QPair<int,int> status = getRelationshipStatus(actionUserId, anotherUserId);

    int id_one = min(actionUserId, anotherUserId);
    int id_two = max(actionUserId, anotherUserId);

    if (status != UNKNOWN_USER) {
        if (!executeQuery(QString("UPDATE relationship SET status = %1, action_user_id = %2 "
                                  "WHERE user_one_id = %3 "
                                  "AND user_two_id = %4").arg(SpUser::Blocked).arg(actionUserId).arg(id_one).arg(id_two))) {
            return;
        }
    } else {
        if (!executeQuery(QString("INSERT INTO relationship (user_one_id,user_two_id,status,action_user_id) "
                                  "VALUES (%1, %2, %3, %4)").arg(id_one).arg(id_two).arg(SpUser::Blocked).arg(actionUserId))) {
            return;
        }
    }

    createEvent(actionUserId, RelationshipChanged);
    createEvent(anotherUserId, RelationshipChanged);
}

QList<QByteArray> SpDatabase::getFriendList(int userId)
{
    QList<QByteArray> friends;
    QList<int> friendsIdList = getFriendsIdList(userId);
    foreach (auto friendId, friendsIdList) {
        SpUser mate = getUserById(friendId);
        mate.mRelatonship = getRelationshipStatus(userId, mate.mId);
        friends.append(mate.toByteArray());
    }
    return friends;
}

QList<int> SpDatabase::getFriendsIdList(int userId)
{
    QueryValuesList list;
    if (!executeQuery(QString("SELECT * "
                              "FROM relationship "
                              "WHERE (user_one_id = %1 OR user_two_id = %2) "
                              "AND (status = %3 OR status = %4)")
                      .arg(userId)
                      .arg(userId)
                      .arg(SpUser::Accepted)
                      .arg(SpUser::Pending), &list)) {
        return QList<int>();
    }

    if (!list.isEmpty()) {
        QList<int> friendsIdList;
        foreach (auto row, list) {
            int user_one_id = row.at(0).toInt();
            int user_two_id = row.at(1).toInt();
            int friendId = (userId == user_one_id) ? user_two_id : user_one_id;
            friendsIdList.append(friendId);
        }
        return friendsIdList;
    } else {
        return QList<int>();
    }
}

QList<QByteArray> SpDatabase::getConversationList(int userId)
{
    QueryValuesList list;
    if (!executeQuery(QString("SELECT conversation_id "
                              "FROM participants "
                              "WHERE user_id = %1")
                      .arg(userId), &list)) {
        return QList<QByteArray>();
    }

    QList<QByteArray> converations;
    foreach (auto row, list) {
        int id = list.at(0).at(0).toInt();
        converations.append(getConversationById(id).toByteArray());
    }

    return converations;
}

QList<int> SpDatabase::getConversationsIdList(int userId)
{
    QueryValuesList list;
    if (!executeQuery(QString("SELECT conversation_id "
                              "FROM participants "
                              "WHERE user_id = %1")
                      .arg(userId), &list)) {
        return QList<int>();
    }

    if (!list.isEmpty()) {
        QList<int> conversationsIdList;
        foreach (auto row, list)
            conversationsIdList.append(row.at(0).toInt());
        return conversationsIdList;
    } else {
        return QList<int>();
    }
}

void SpDatabase::createEvent(int userId, int event)
{
    QueryValuesList list;
    if (!executeQuery(QString("SELECT * FROM events "
                              "WHERE target_user_id = %1 "
                              "AND type = %2").arg(userId).arg(event), &list)) {
        return;
    }    

    if (list.isEmpty()) {
        if (!executeQuery(QString("INSERT INTO events (target_user_id,type) "
                                  "VALUES (%1,%2)").arg(userId).arg(event))) {
            return;
        }
    }
}

void SpDatabase::removeEvent(int userId, int event)
{
    if (!executeQuery(QString("DELETE FROM events "
                              "WHERE target_user_id = %1 "
                              "AND type = %2").arg(userId).arg(event))) {
        return;
    }
}

int SpDatabase::checkEvent(int userId)
{
    QueryValuesList list;
    if (!executeQuery(QString("SELECT type FROM events "
                              "WHERE target_user_id=%1").arg(userId), &list)) {
        return SpPackage::ERR;
    }

    if (!list.empty()) {
        int eventType = list.at(0).at(0).toInt();
        removeEvent(userId, eventType);
        return eventType;
    } else {
        return SpPackage::EMPTY;
    }
}

QList<QByteArray> SpDatabase::getNewMessages(int userId)
{
    QList<int> conversationsIdList = getConversationsIdList(userId);
    if (conversationsIdList.isEmpty())
        return QList<QByteArray>();

    qint64 lastUpdate = getUserLastUpdate(userId);
    if (lastUpdate < 0)
        return QList<QByteArray>();

    QList<QByteArray> messageList;
    foreach (int conversationId, conversationsIdList) {
        QueryValuesList list;
        if (!executeQuery(QString("SELECT * "
                                  "FROM messages "
                                  "WHERE conversation_id = %1 "
                                  "AND created_at > %2").arg(conversationId).arg(lastUpdate), &list)) {
            return QList<QByteArray>();
        }

        foreach (auto row, list) {
            SpMessage msg;
            msg.mSenderId = row.at(1).toInt();
            msg.mConversationId = row.at(2).toInt();
            msg.mDateTime.setTime_t(uint(row.at(3).toInt()));
            msg.mPayload.mTextMessage = row.at(4).toString();
            int messageId = row.at(0).toInt();

            QueryValuesList imageList;
            if (!executeQuery(QString("SELECT path_to_file FROM images "
                                      "WHERE message_id = %1").arg(messageId), &imageList)) {
                return QList<QByteArray>();
            }
            foreach (auto imageRow, imageList) {
                QImage image;
                QString path = imageRow.value(0).toString();
                if (!image.load(path)) {
                    ERROR_LOG("[SpDatabase] Невозможно загрузить картинку из файла [" + path + "].");
                    return QList<QByteArray>();
                }
                msg.mPayload.mPictures.append(image);
            }

            messageList.append(msg.toByteArray());
        }
    }


    if (!setUserLastUpdate(userId, QDateTime::currentDateTime().toTime_t())) {
        return QList<QByteArray>();
    }

    return messageList;
}

qint64 SpDatabase::getUserLastUpdate(int userId)
{
    QueryValuesList list;
    if (!executeQuery(QString("SELECT last_update FROM users "
                              "WHERE id = %1").arg(userId), &list)) {
        return -1;
    }

    qint64 lastUpdate;
    if (!list.isEmpty()) {
        lastUpdate = list.at(0).at(0).toInt();
        return lastUpdate;
    } else {
        return -1;
    }
}

bool SpDatabase::setUserLastUpdate(int userId, qint64 lastUpdate)
{
    if (!executeQuery(QString("UPDATE users "
                              "SET last_update = %1 "
                              "WHERE id = %2").arg(lastUpdate).arg(userId))) {
        return false;
    } else {
        return true;
    }
}

SpUser SpDatabase::getUserById(int userId)
{
    QueryValuesList list;
    if (!executeQuery(QString("SELECT * FROM users "
                              "WHERE id = %1").arg(userId), &list)) {
        return SpUser();
    }

    SpUser user;
    foreach (auto row, list) {
        user.mId = row.at(0).toInt();
        user.mName = row.at(4).toString();
        user.mEmail = row.at(3).toString();
        user.mStatus = row.at(8).toInt();
        QString path = row.at(5).toString();
        if (!user.mAvatar.load(path)) {
            ERROR_LOG("[SpDatabase] Невозможно загрузить картинку из файла [" + path + "].");
            return SpUser();
        }
    }

    return user;
}

QList<QByteArray> SpDatabase::getUserListByName(QString name, int searchingUserId)
{
   QList<QByteArray> userList;

    if (!name.isEmpty()) {
        QueryValuesList list;
        if (!executeQuery(QString("SELECT * FROM users "
                                  "WHERE name LIKE '%%1%'").arg(name), &list)) {
            return QList<QByteArray>();
        }
        foreach (auto row, list) {
            if (row.at(0).toInt() != searchingUserId) {
                SpUser user;
                user.mId = row.at(0).toInt();
                user.mName = row.at(4).toString();
                user.mEmail = row.at(3).toString();
                user.mStatus = row.at(8).toInt();
                user.mRelatonship = getRelationshipStatus(searchingUserId, user.mId);
                QString path = row.at(5).toString();
                if (!user.mAvatar.load(path)) {
                    ERROR_LOG("[SpDatabase] Невозможно загрузить картинку из файла [" + path + "].");
                    return QList<QByteArray>();
                }
                userList.append(user.toByteArray());
            }
        }
    }

    return userList;
}

SpConversation SpDatabase::getConversationById(int id)
{
    QueryValuesList list;
    if (!executeQuery(QString("SELECT * FROM conversation "
                              "WHERE id = %1").arg(id), &list)) {
        return SpConversation();
    }

    if (list.isEmpty()) {
        return SpConversation();
    } else {
        SpConversation conversation;
        //Общая информация
        conversation.mCreatorId = list.at(0).at(2).toInt();
        conversation.mTitle = list.at(0).at(1).toString();
        conversation.mCreatedAt = list.at(0).at(3).toDateTime();
        conversation.mUpdatedAt = list.at(0).at(4).toDateTime();
        conversation.mType = list.at(0).at(5).toInt();
        int conversationId = list.at(0).at(0).toInt();
        conversation.mId = conversationId;
        //Получение списка участников
        QList<int> participantIdList = getParticipantIdList(conversationId);
        foreach (auto participantId, participantIdList) {
            conversation.mUsers.append(getUserById(participantId));
        }
        //Получение списка сообщений
        if (!executeQuery(QString("SELECT * FROM messages "
                                  "WHERE conversation_id = %1").arg(conversationId), &list)) {
            return SpConversation();
        }
        foreach (auto row, list) {
            SpMessage msg;
            msg.mSenderId = row.at(1).toInt();
            msg.mConversationId = row.at(2).toInt();
            msg.mDateTime = QDateTime::fromTime_t(uint(row.at(3).toInt()));
            msg.mPayload.mTextMessage = row.at(4).toString();
            int messageId = row.at(0).toInt();

            QueryValuesList imageList;
            if (!executeQuery(QString("SELECT path_to_file FROM images "
                                      "WHERE message_id = %1").arg(messageId), &imageList)) {
                return SpConversation();
            }
            foreach (auto imageRow, imageList) {
                QImage image;
                QString path = imageRow.value(0).toString();
                if (!image.load(path)) {
                    ERROR_LOG("[SpDatabase] Невозможно загрузить картинку из файла [" + path + "].");
                    return SpConversation();
                }
                msg.mPayload.mPictures.append(image);
            }
            conversation.addMessage(msg);
        }
        return conversation;
    }
}

QList<int> SpDatabase::getParticipantIdList(int conversationId)
{
    QueryValuesList list;
    if (!executeQuery(QString("SELECT user_id FROM participants "
                              "WHERE conversation_id = %1").arg(conversationId), &list)) {
        return QList<int>();
    }
    QList<int> participantList;
    foreach (auto row, list) {
        participantList.append(row.at(0).toInt());
    }
    return participantList;
}

void SpDatabase::addMessage(SpMessage message)
{
    if (!executeQuery(QString("INSERT INTO messages (sender_id,conversation_id,created_at,text_message) "
                              "VALUES (%1,%2,%3,'%4')")
                      .arg(message.mSenderId)
                      .arg(message.mConversationId)
                      .arg(message.mDateTime.toTime_t())
                      .arg(message.mPayload.mTextMessage))) {
        return;
    }

    //Получение id добавленного сообщения
    QueryValuesList list;
    if (!executeQuery(QString("SELECT id FROM messages "
                              "WHERE sender_id = %1 "
                              "AND conversation_id = %2 "
                              "AND created_at = %3")
                      .arg(message.mSenderId)
                      .arg(message.mConversationId)
                      .arg(message.mDateTime.toTime_t()), &list)) {
        return;
    }
    if (list.isEmpty()) {
        ERROR_LOG("[SpDatabase] Запись о только что созданном сообщении не найдена.");
        return;
    }

    int messageId = list.at(0).at(0).toInt();
    for (int i = 0; i < message.mPayload.mPictures.size(); ++i) {
        QImage image = message.mPayload.mPictures.at(i);
        QString pathToFile = PATTERN_MSG_IMAGE_PATH(messageId, i);
        if (!saveImageToFile(image, pathToFile)) {
            return;
        }
        if (!executeQuery(QString("INSERT INTO images (message_id,path_to_file) "
                                  "VALUES (%1,'%2')").arg(messageId).arg(pathToFile))) {
            return;
        }
    }

    QList<int> participantIdList = getParticipantIdList(message.mConversationId);
    foreach (auto participantId, participantIdList) {
        createEvent(participantId, NewMessagesReceived);
    }
}

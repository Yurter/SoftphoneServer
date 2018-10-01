#ifndef SPDATABASEHELPER_H
#define SPDATABASEHELPER_H

#include <QRegularExpression>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "SpConversation.h"
#include "SpPackage.h"
#include "SpMessage.h"
#include "SpUser.h"

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define LOG_FAILED_QUERY(query) ERROR_LOG("[LOG_FAILED_QUERY]\n" \
                                + QString(__FILE__) + " " + QString(__LINE__) + "\n" \
                                "FAILED QUERY:\n"\
                                "[ " + (query).executedQuery() + " ]\n"\
                                + (query).lastError().text())
#define UNKNOWN_USER QPair<int,int>(SpUser::Unknown,-1)
#define INVALID_RELATIONSHIP QPair<int,int>(-1,-1)
#define PATTERN_AVATAR_PATH(userId) ("images/img_" + QString::number(userId) + ".png")
#define PATTERN_MSG_IMAGE_PATH(messageId,imageId) ("images/img_" + QString::number(messageId) + "_" + QString::number(imageId) + ".png")

typedef QList<QList<QVariant>> QueryValuesList;

class SpDatabase : public QObject
{
    Q_OBJECT
public:
    explicit SpDatabase(QObject *parent = nullptr);

    void                setDatabasePath(QString path);
    bool                openDatabase();
    void                clearDatabase();

    int                 createAccaunt(SpUser  user, QString login, QString password);
    int                 verifyAccount(SpUser *user, QString login, QString password);
    void                deleteAccaunt(SpUser user);

    QList<QByteArray>   getFriendList(int userId);
    QList<QByteArray>   getConversationList(int userId);
    int                 checkEvent(int userId);

    void                addUserToFriends(int actionUserId, int anotherUserId);
    void                removeUserFromFriends(int actionUserId, int anotherUserId);
    void                blockUser(int actionUserId, int anotherUserId);

    void                addMessage(SpMessage message);
    QList<QByteArray>   getNewMessages(int userId);
    SpConversation      createConversation(int creatorId, QString title, int type);
    SpConversation      createDialog(int userOneId, int userTwoId);
    void                addParticipantToConversation(int userId, int userType, int conversationId);

    void                changeUserStatus(int userId, int newStatus);
    QList<QByteArray>   getUserListByName(QString name, int searchingUserId);

private:
    SpUser              getUserById(int id);
    SpConversation      getConversationById(int id);
    qint64              getUserLastUpdate(int userId);
    bool                setUserLastUpdate(int userId, qint64 lastUpdate);
    QList<int>          getFriendsIdList(int userId);
    QList<int>          getConversationsIdList(int userId);
    QList<int>          getParticipantIdList(int conversationId);

private:
    void                createEvent(int userId, int event);
    void                removeEvent(int userId, int event);

    bool                verifyLogin(QString login);
    bool                passwordCorrect(QString password);

    bool                setRelationshipStatus(int userOneId, int userTwoId, int status);
    QPair<int,int>      getRelationshipStatus(int userOneId, int userTwoId);

    bool                saveImageToFile(QImage image, QString path);
    bool                executeQuery(QString query, QueryValuesList *result = nullptr);

private:
    QSqlDatabase        mDatabase;
    QString             mDatabasePath;

public:
    enum Event
    {
        NewMessagesReceived,
        RelationshipChanged,
        FriendStatusChanged,
        ChangesInConversations
    };
};

#endif // SPDATABASEHELPER_H

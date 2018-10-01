#include "SpTcpServer.h"

SpTcpServer::SpTcpServer(QObject *parent) :
    QTcpServer(parent)
{
    DEBUG_LOG("Создан объект класса SpTcpServer.");
}

void SpTcpServer::setPort(quint16 port)
{
    DEBUG_LOG("[SpTcpServer] Установлен порт сервера: " + QString::number(port));
    mPort = port;
}

void SpTcpServer::setDatabasePath(QString path)
{
    DEBUG_LOG("[SpTcpServer] Установлено имя файла базы данных: " + path);
    mDatabase.setDatabasePath(path);
}

SpTcpServer::~SpTcpServer()
{
    DEBUG_LOG("Уничтожен объект класса SpTcpServer.");
}

void SpTcpServer::start()
{
    if (mDatabase.openDatabase()) {
        if (listen(QHostAddress::Any, mPort)) {
            INFO_LOG("[SpTcpServer] TCP Server listen port: " + QString::number(mPort));
            INFO_LOG("[SpTcpServer] TCP Server is running.");
        } else {
            ERROR_LOG("[SpTcpServer] TCP Server could not start: Could not start listen incoming connection.");
            QCoreApplication::exit(ListenError);
        }
    } else {
        ERROR_LOG("[SpTcpServer] TCP Server could not start: Could not open database.");
        QCoreApplication::exit(DatabaseError);
    }
}

void SpTcpServer::incomingConnection(qintptr handle)
{
    DEBUG_LOG("[SpTcpServer] Входящее соединение.");
    SpTcpServerSocket *socket = new SpTcpServerSocket(handle);
    connect(socket, SIGNAL(processTheData(QByteArray)), this, SLOT(onProcessTheData(QByteArray)));
    connect(this, SIGNAL(readyToResponse(QByteArray)), socket, SLOT(onReadyToResponse(QByteArray)));
    connect(socket, SIGNAL(finished()), socket, SLOT(deleteLater()));
    socket->start();
}

void SpTcpServer::onProcessTheData(QByteArray data)
{
    SpPackage request(data);

    SpPackage response;
    response.mSenderId = SERVER_ID;
    response.mReceiverId = request.mSenderId;

    if (!request.isValide()) {
        ERROR_LOG("[SpTcpServer] Получен невалидный пакет с типом " + QString::number(request.mType));
        response.mType = SpPackage::ERR;
        emit readyToResponse(response.toByteArray());
        return;
    }

    switch (request.mType) {
    case SpPackage::SIGN_IN: {
        SpPackage::SignInData data(request.mTrunk.at(0));
        SpUser user;
        int result = mDatabase.verifyAccount(&user, data.login, data.password);
        response.mType = result;
        if (user.isValid()) {
            response.mReceiverId = user.mId;
            response.mTrunk.append(user.toByteArray());
        }
        break;
    }
    case SpPackage::SIGN_UP: {
        SpPackage::SignUpData data(request.mTrunk.at(0));
        response.mType = mDatabase.createAccaunt(SpUser(data.user), data.login, data.password);
        break;
    }
    case SpPackage::CHECK_FOR_EVENTS: {
        int event = mDatabase.checkEvent(request.mSenderId);
        switch (event) {
        case SpDatabase::NewMessagesReceived: {
            response.mTrunk = mDatabase.getNewMessages(request.mSenderId);
            response.mType = SpPackage::NEW_MESSAGE_LIST;
        }
            break;
        case SpDatabase::RelationshipChanged: {
            response.mTrunk = mDatabase.getFriendList(request.mSenderId);
            response.mType = SpPackage::FRIEND_LIST;
        }
            break;
        case SpDatabase::FriendStatusChanged: {
            response.mTrunk = mDatabase.getFriendList(request.mSenderId);
            response.mType = SpPackage::FRIEND_LIST;
        }
            break;
        case SpDatabase::ChangesInConversations: {
            response.mTrunk = mDatabase.getConversationList(request.mSenderId);
            response.mType = SpPackage::CONVERSATION_LIST;
        }
            break;
        default:
            response.mType = SpPackage::EMPTY;
            break;
        }
        break;
    }
    case SpPackage::REQUEST_FRIEND_LIST: {
        response.mTrunk = mDatabase.getFriendList(request.mSenderId);
        response.mType = SpPackage::FRIEND_LIST;
        break;
    }
    case SpPackage::REQUEST_CONVERSATION_LIST: {
        QList<QByteArray> list = mDatabase.getConversationList(request.mSenderId);
        if (list.isEmpty()) {
            response.mType = SpPackage::EMPTY;
        } else {
            response.mTrunk = list;
            response.mType = SpPackage::CONVERSATION_LIST;
        }
        break;
    }
    case SpPackage::FIND_USER_BY_NAME: {
        SpPackage::UserSearchData data(request.mTrunk.at(0));
        QList<QByteArray> list = mDatabase.getUserListByName(data.userName, data.searchingUserId);
        if (list.isEmpty()) {
            response.mType = SpPackage::EMPTY;
        } else {
            response.mTrunk = list;
            response.mType = SpPackage::USER_SEARCH_RESULT;
        }
        break;
    }
    case SpPackage::STATUS_CHANGED: {
        int newStatus = request.mTrunk.at(0).toInt();
        mDatabase.changeUserStatus(request.mSenderId, newStatus);
        response.mType = SpPackage::EMPTY;
        break;
    }
    case SpPackage::ADD_TO_FRIENDS: {
        int otherUserId = request.mTrunk.at(0).toInt();
        mDatabase.addUserToFriends(request.mSenderId, otherUserId);
        response.mTrunk = mDatabase.getFriendList(request.mSenderId);
        response.mType = SpPackage::FRIEND_LIST;
        break;
        }
    case SpPackage::REMOVE_FROM_FRIENDS: {
        int otherUserId = request.mTrunk.at(0).toInt();
        mDatabase.removeUserFromFriends(request.mSenderId, otherUserId);
        response.mTrunk = mDatabase.getFriendList(request.mSenderId);
        response.mType = SpPackage::FRIEND_LIST;
        break;
    }
    case SpPackage::BLOCK_USER: {
        int otherUserId = request.mTrunk.at(0).toInt();
        mDatabase.blockUser(request.mSenderId, otherUserId);
        response.mType = SpPackage::EMPTY;
        break;
    }
    case SpPackage::MESSAGE: {
        mDatabase.addMessage(SpMessage(request.mTrunk.at(0)));
        response.mType = SpPackage::EMPTY;
        break;
    }
    default:
        response.mType = SpPackage::EMPTY;
        break;
    }

    if (!response.isValide()) {
        ERROR_LOG("[SpTcpServer] Попытка отправки невалидного пакета с типом " + QString::number(response.mType));
        response.mType = SpPackage::ERR;
        return;
    }

    emit readyToResponse(response.toByteArray());
}

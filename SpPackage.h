#ifndef SPPACKAGE_H
#define SPPACKAGE_H

#include <QDataStream>
#include "Logger.h"

class SpPackage : public QObject
{
    Q_OBJECT
public:
    explicit SpPackage(QObject *parent = nullptr);
    explicit SpPackage(QByteArray array, QObject *parent = nullptr);
    SpPackage(const SpPackage &obj);

    bool        isValide();
    QByteArray  toByteArray();
    bool        isThisForMe(int myid);

    class SignInData {
    public:
        SignInData() {}
        explicit SignInData(QByteArray array)
        {
            QDataStream stream(array);
            stream >> *this;
        }
        SignInData(QString log, QString pas) :
            login(log),
            password(pas) {}
        QString login;
        QString password;

        QByteArray  toByteArray() {
            QByteArray array;
            QDataStream stream(&array, QIODevice::WriteOnly);
            stream << *this;
            return array;
        }

        friend QDataStream &operator<<(QDataStream &out, const SignInData &obj)
        {
            out << obj.login << obj.password;
            return out;
        }

        friend QDataStream &operator>>(QDataStream &in, SignInData &obj)
        {
            in >> obj.login >> obj.password;
            return in;
        }
    };

    class SignUpData {
    public:
        SignUpData() {}
        explicit SignUpData(QByteArray array)
        {
            QDataStream stream(array);
            stream >> *this;
        }
        SignUpData(QString log, QString pas, QByteArray u) :
            login(log),
            password(pas),
            user(u) {}

        QString     login;
        QString     password;
        QByteArray  user;

        QByteArray  toByteArray() {
            QByteArray array;
            QDataStream stream(&array, QIODevice::WriteOnly);
            stream << *this;
            return array;
        }

        friend QDataStream &operator<<(QDataStream &out, const SignUpData &obj)
        {
            out << obj.login << obj.password << obj.user;
            return out;
        }

        friend QDataStream &operator>>(QDataStream &in, SignUpData &obj)
        {
            in >> obj.login >> obj.password >> obj.user;
            return in;
        }
    };

    class UserSearchData {
    public:
        UserSearchData() :
            searchingUserId(-1){}
        explicit UserSearchData(QByteArray array)
        {
            QDataStream stream(array);
            stream >> *this;
        }
        UserSearchData(QString name, int id) :
            userName(name),
            searchingUserId(id){}

        QString userName;
        int     searchingUserId;

        QByteArray  toByteArray() {
            QByteArray array;
            QDataStream stream(&array, QIODevice::WriteOnly);
            stream << *this;
            return array;
        }

        friend QDataStream &operator<<(QDataStream &out, const UserSearchData &obj)
        {
            out << obj.userName << obj.searchingUserId;
            return out;
        }

        friend QDataStream &operator>>(QDataStream &in, UserSearchData &obj)
        {
            in >> obj.userName >> obj.searchingUserId;
            return in;
        }
    };

    enum PackageType {
        // Коды запросов
        SIGN_IN,
        SIGN_UP,
        MESSAGE,
        CHECK_FOR_EVENTS,
        FIND_USER_BY_NAME,
        REQUEST_FRIEND_LIST,
        REQUEST_CONVERSATION_LIST,
        // Коды ответов
        SUCCESS_SIGN_IN,
        FAIL_SIGN_IN,
        SUCCESS_SIGN_UP,
        FAIL_SIGN_UP_BAD_LOGIN,
        FAIL_SIGN_UP_BAD_PASSWORD,
        NEW_MESSAGE_LIST,
        USER_SEARCH_RESULT,
        FRIEND_LIST,
        CONVERSATION_LIST,
        // Коды запросов без ответа
        ADD_TO_FRIENDS,
        REMOVE_FROM_FRIENDS,
        BLOCK_USER,
        STATUS_CHANGED,
        // Остальные ответы
        ERR,
        EMPTY
    };

    // Компоненты пакета
    int                 mSenderId;
    int                 mReceiverId;
    int                 mType;
    QList<QByteArray>   mTrunk;

    friend QDataStream &operator<<(QDataStream &out, const SpPackage &obj)
    {
        out << obj.mSenderId << obj.mReceiverId << obj.mType << obj.mTrunk;
        return out;
    }

    friend QDataStream &operator>>(QDataStream &in, SpPackage &obj)
    {
        in >> obj.mSenderId >> obj.mReceiverId >> obj.mType >> obj.mTrunk;
        return in;
    }

    SpPackage& operator=(const SpPackage &obj)
    {
        mSenderId = obj.mSenderId;
        mReceiverId = obj.mReceiverId;
        mType = obj.mType;
        mTrunk = obj.mTrunk;
        return *this;
    }
};

#endif // SPPACKAGE_H

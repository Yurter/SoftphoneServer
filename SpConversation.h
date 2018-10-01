#ifndef SPCONVERSATION_H
#define SPCONVERSATION_H

#include "SpUser.h"
#include "SpMessage.h"

class SpConversation : public QObject
{
    Q_OBJECT
public:
    explicit SpConversation(QObject *parent = nullptr);
    explicit SpConversation(SpConversation *conversation);
    explicit SpConversation(QByteArray array, QObject *parent = nullptr);
    SpConversation(const SpConversation &obj);

    bool            isValid();
    QByteArray      toByteArray();

    void            addMessage(SpMessage message);

    enum ConversationType {
        Polylogue,
        Dialog
    };

    int                 mId;
    int                 mCreatorId;
    QString             mTitle;
    QDateTime           mCreatedAt;
    QDateTime           mUpdatedAt;
    int                 mType;
    QList<SpUser>       mUsers;
    QImage              mPicture;
    QList<SpMessage>    mMessages;
    int                 mUnreadMessagesCount;

    friend QDataStream &operator<<(QDataStream &out, const SpConversation &obj)
    {
        out << obj.mId
            << obj.mCreatorId
            << obj.mTitle
            << obj.mCreatedAt
            << obj.mUpdatedAt
            << obj.mType
            << obj.mUsers
            << obj.mPicture
            << obj.mMessages
            << obj.mUnreadMessagesCount;
        return out;
    }

    friend QDataStream &operator>>(QDataStream &in, SpConversation &obj)
    {
        in >> obj.mId
           >> obj.mCreatorId
           >> obj.mTitle
           >> obj.mCreatedAt
           >> obj.mUpdatedAt
           >> obj.mType
           >> obj.mUsers
           >> obj.mPicture
           >> obj.mMessages
           >> obj.mUnreadMessagesCount;
        return in;
    }

    SpConversation& operator=(const SpConversation &obj)
    {
        mId = obj.mId;
        mCreatorId = obj.mCreatorId;
        mTitle = obj.mTitle;
        mCreatedAt = obj.mCreatedAt;
        mUpdatedAt = obj.mUpdatedAt;
        mType = obj.mType;
        mUsers = obj.mUsers;
        mPicture = obj.mPicture;
        mMessages = obj.mMessages;
        mUnreadMessagesCount = obj.mUnreadMessagesCount;
        return *this;
    }
};

#endif // SPCONVERSATION_H

#include "SpConversation.h"

SpConversation::SpConversation(QObject *parent) :
    QObject(parent)
{
    mId = -1;
}

SpConversation::SpConversation(SpConversation *conversation)
{
    *this = *conversation;
}

SpConversation::SpConversation(QByteArray array, QObject *parent) :
    QObject(parent)
{
    QDataStream stream(array);
    stream >> *this;
}

SpConversation::SpConversation(const SpConversation &obj)
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
}

bool SpConversation::isValid()
{
    if (mId < 0 ||
            mCreatorId < 0 ||
            mTitle.isEmpty() ||
            !mCreatedAt.isValid() ||
            !mUpdatedAt.isValid() ||
            !(mType == Polylogue || mType == Dialog) ||
            mUsers.isEmpty() ||
            mPicture.isNull() ||
            mUnreadMessagesCount < 0) {
        return false;
    } else {
        return true;
    }
}

QByteArray SpConversation::toByteArray()
{
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream << *this;
    return array;
}

void SpConversation::addMessage(SpMessage message)
{
    mMessages.append(message);
}

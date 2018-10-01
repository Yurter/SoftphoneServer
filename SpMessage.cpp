#include "SpMessage.h"

SpMessage::SpMessage(QObject *parent) :
    QObject(parent)
{
    mSenderId = -1;
    mConversationId = -1;
    mDateTime = QDateTime::currentDateTime();
}

SpMessage::SpMessage(SpMessage *message)
{
    *this = *message;
}

SpMessage::SpMessage(QByteArray array, QObject *parent) :
    QObject(parent)
{
    QDataStream stream(array);
    stream >> *this;
}

SpMessage::SpMessage(const SpMessage &obj)
{
    mSenderId = obj.mSenderId;
    mConversationId = obj.mConversationId;
    mDateTime = obj.mDateTime;
    mPayload = obj.mPayload;
}

bool SpMessage::isValide()
{
    if (mSenderId < 0 ||
            mConversationId < 0 ||
            !mDateTime.isValid()) {
        return false;
    } else {
        return true;
    }
}

QByteArray SpMessage::toByteArray()
{
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream << *this;
    return array;
}

#include "SpPackage.h"

SpPackage::SpPackage(QObject *parent) :
    QObject(parent),
    mSenderId(-1),
    mReceiverId(-1),
    mType(-1)
{

}

SpPackage::SpPackage(QByteArray array, QObject *parent) :
    QObject(parent)
{
    QDataStream stream(array);
    stream >> *this;
}

SpPackage::SpPackage(const SpPackage &obj)
{
    mSenderId = obj.mSenderId;
    mReceiverId = obj.mReceiverId;
    mType = obj.mType;
    mTrunk = obj.mTrunk;
}

bool SpPackage::isValide()
{
    //Айди отправителя и получателя могут быть меньше нуля, только в особых случаях
    bool typeExeptionOne = (mType == SIGN_IN ||
                            mType == SIGN_UP);
    bool typeExeptionTwo = (mType == ERR ||
                            mType == FAIL_SIGN_IN ||
                            mType == SUCCESS_SIGN_UP ||
                            mType == FAIL_SIGN_UP_BAD_LOGIN ||
                            mType == FAIL_SIGN_UP_BAD_PASSWORD);
    if (!typeExeptionOne && mSenderId < 0)
        return false;
    if (!typeExeptionTwo && mReceiverId < 0)
        return false;

    switch (mType) {
    case SIGN_IN:
    case SIGN_UP:
    case MESSAGE:
    case FIND_USER_BY_NAME:
    case SUCCESS_SIGN_IN:
    case ADD_TO_FRIENDS:
    case REMOVE_FROM_FRIENDS:
    case BLOCK_USER:
    case STATUS_CHANGED:
        return mTrunk.size() == 1;
    case SUCCESS_SIGN_UP:
    case CHECK_FOR_EVENTS:
    case REQUEST_FRIEND_LIST:
    case REQUEST_CONVERSATION_LIST:
    case FAIL_SIGN_IN:
    case FAIL_SIGN_UP_BAD_LOGIN:
    case FAIL_SIGN_UP_BAD_PASSWORD:
    case EMPTY:
    case ERR:
        return mTrunk.isEmpty();
    case NEW_MESSAGE_LIST:
        return !mTrunk.isEmpty();
    case USER_SEARCH_RESULT:
    case FRIEND_LIST:
    case CONVERSATION_LIST:
        return true;
    default:
        return false;
    }
}

QByteArray SpPackage::toByteArray()
{
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream << *this;
    return array;
}

bool SpPackage::isThisForMe(int myId)
{
    switch (mType) {
    case SUCCESS_SIGN_IN:
    case SUCCESS_SIGN_UP:
    case FAIL_SIGN_IN:
    case FAIL_SIGN_UP_BAD_LOGIN:
    case FAIL_SIGN_UP_BAD_PASSWORD:
    case EMPTY:
    case ERR:
        return true;
        break;
    }
    return mReceiverId == myId;
}

#include "SpUser.h"

SpUser::SpUser(QObject *parent) :
    QObject(parent)
{
    mId = -1;
    mStatus = -1;
    mRelatonship = QPair<int,int>(-1,-1);

    if (parent)
        *this = *(SpUser*)parent;
}

SpUser::SpUser(SpUser *user)
{
    *this = *user;
}

SpUser::SpUser(QByteArray array, QObject *parent) :
    QObject(parent)
{
    QDataStream stream(array);
    stream >> *this;
}

SpUser::SpUser(const SpUser &obj)
{
    mId = obj.mId;
    mName = obj.mName;
    mEmail = obj.mEmail;
    mStatus = obj.mStatus;
    mAvatar = obj.mAvatar;
    mRelatonship = obj.mRelatonship;
}

QByteArray SpUser::toByteArray()
{
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream << *this;
    return array;
}

bool SpUser::isValid()
{
    return (mId > -1) &&
           (mStatus > -1) &&
           !mName.isEmpty();
}

#ifndef SPUSER_H
#define SPUSER_H

#include <QPair>
#include <QImage>
#include <QDataStream>
#include <QDebug>

class SpUser : public QObject
{
    Q_OBJECT
public:
    explicit SpUser(QObject *parent = nullptr);
    explicit SpUser(SpUser *user);
    explicit SpUser(QByteArray array, QObject *parent = nullptr);
    SpUser(const SpUser &obj);

    QByteArray  toByteArray();

    int             mId;
    QString         mName;
    QString         mEmail;
    int             mStatus;
    QImage          mAvatar;

    QPair<int,int>  mRelatonship;

    bool            isValid();

    enum Status {
        Online,
        Offline,
        Away,
        DoNotDisturb,
        Invisible,
        Deleted
    };

    enum RelationshipType {
        Unknown,
        Pending,
        Accepted,
        Blocked
    };

    enum ParticipantType {
        Creator,
        Invited
    };

    friend QDataStream &operator<<(QDataStream &out, const SpUser &obj)
    {
        out << obj.mId << obj.mName << obj.mEmail << obj.mStatus << obj.mAvatar << obj.mRelatonship;
        return out;
    }

    friend QDataStream &operator>>(QDataStream &in, SpUser &obj)
    {
        in >> obj.mId >> obj.mName >> obj.mEmail >> obj.mStatus >> obj.mAvatar >> obj.mRelatonship;
        return in;
    }

    SpUser& operator=(const SpUser &obj)
    {
        mId = obj.mId;
        mName = obj.mName;
        mEmail = obj.mEmail;
        mStatus = obj.mStatus;
        mAvatar = obj.mAvatar;
        mRelatonship = obj.mRelatonship;
        return *this;
    }
};

#endif // SPUSER_H

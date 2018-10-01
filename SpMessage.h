#ifndef SPMESSAGE_H
#define SPMESSAGE_H

#include <QImage>
#include <QDateTime>
#include <QDataStream>

class SpMessage : public QObject
{
    Q_OBJECT
public:
    explicit SpMessage(QObject *parent = nullptr);
    explicit SpMessage(SpMessage *message);
    explicit SpMessage(QByteArray array, QObject *parent = nullptr);
    SpMessage(const SpMessage &obj);

    bool        isValide();
    QByteArray  toByteArray();

    class Payload
    {
    public:
        QString         mTextMessage;
        QList<QImage>   mPictures;
//      QList<QSound>   mAudios;
//      QList<Video>    mVideos;
//      QList<Document> mDocuments;

//        Payload& operator=(const Payload &obj)
//        {
//            mTextMessage = obj.mTextMessage;
//            mPictures = obj.mPictures;
//            return *this;
//        }
    };

// Компоненты сообщения
    int             mSenderId;
    int             mConversationId;
    QDateTime       mDateTime;
    Payload         mPayload;

    friend QDataStream &operator<<(QDataStream &out, const SpMessage &obj)
    {
        out << obj.mSenderId
            << obj.mConversationId
            << obj.mDateTime
            << obj.mPayload.mTextMessage
            << obj.mPayload.mPictures;
        return out;
    }

    friend QDataStream &operator>>(QDataStream &in, SpMessage &obj)
    {
        in >> obj.mSenderId
           >> obj.mConversationId
           >> obj.mDateTime
           >> obj.mPayload.mTextMessage
           >> obj.mPayload.mPictures;
        return in;
    }

    SpMessage& operator=(const SpMessage &obj)
    {
        mSenderId = obj.mSenderId;
        mConversationId = obj.mConversationId;
        mDateTime = obj.mDateTime;
        mPayload = obj.mPayload;
        return *this;
    }
};

#endif // SPMESSAGE_H

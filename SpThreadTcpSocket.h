#ifndef SPTHREADTCPSOCKET_H
#define SPTHREADTCPSOCKET_H

#include <QThread>
#include <QTcpSocket>
#include <QHostAddress>

class SpThreadTcpSocket : public QThread
{
    Q_OBJECT
public:
    explicit SpThreadTcpSocket(QObject *parent = nullptr);
    ~SpThreadTcpSocket();

signals:
    void            processTheData(QByteArray data);

public slots:
    void            onDisconnected();


protected:
    QTcpSocket     *mTcpSocket;
    QByteArray      mData;
};

#endif // SPTHREADTCPSOCKET_H

#ifndef SPTHREADUDPSOCKET_H
#define SPTHREADUDPSOCKET_H

#include <QThread>
#include <QUdpSocket>
#include <QHostAddress>
#include <QNetworkDatagram>

class SpThreadUdpSocket : public QThread
{
    Q_OBJECT
public:
    explicit SpThreadUdpSocket(QHostAddress address, quint16 port, QObject *parent = nullptr);
    explicit SpThreadUdpSocket(qintptr descriptor, QObject *parent = nullptr);
    ~SpThreadUdpSocket();

    void            run();
    void            sendDatagram(QNetworkDatagram datagram);

public slots:
    void            readPendingDatagrams();

signals:
    void            processTheDatagram(QNetworkDatagram datagram);

protected:
    QUdpSocket     *mUdpSocket;
    QHostAddress    mAddress;
    quint16         mPort;
};

#endif // SPTHREADUDPSOCKET_H

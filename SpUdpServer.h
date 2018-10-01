#ifndef SPUDPSERVER_H
#define SPUDPSERVER_H

#include <QCoreApplication>
#include "SpDatabase.h"
#include "SpThreadUdpSocket.h"

#define SERVER_ID 0

class SpUdpServer : public QObject
{
    Q_OBJECT
public:
    explicit SpUdpServer(QObject *parent = nullptr);
    ~SpUdpServer();

    void                start();

protected:
    void                incomingConnection(qintptr handle);
    bool                listen(QHostAddress address = QHostAddress::Any, quint16 port = 0);

public:
    void                setPort(quint16 port);
    void                setDatabasePath(QString path);

signals:
    void                readyToResponse(QNetworkDatagram datagram);

public slots:
    void                onDatagramReceived(QNetworkDatagram datagram);

private:
    SpThreadUdpSocket  *mSocket;
    SpDatabase          mDatabase;

    quint16             mPort;

    enum Error {
        UdpError = 1,
        ListenError,
        DatabaseError
    };
};

#endif // SPUDPSERVER_H

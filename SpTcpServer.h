#ifndef SPTCPSERVER_H
#define SPTCPSERVER_H

#include <QTcpServer>
#include <QCoreApplication>
#include "SpTcpServerSocket.h"
#include "SpDatabase.h"
#include "SpMessage.h"
#include "SpPackage.h"
#include "SpUser.h"

#define SERVER_ID 0

class SpTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit SpTcpServer(QObject *parent = nullptr);
    ~SpTcpServer();

    void                start();

protected:
    void                incomingConnection(qintptr handle);

public:
    void                setPort(quint16 port);
    void                setDatabasePath(QString path);

signals:
    void                readyToResponse(QByteArray data);

public slots:
    void                onProcessTheData(QByteArray data);

private:
    SpDatabase          mDatabase;
    quint16             mPort;

    enum Error {
        ListenError,
        DatabaseError
    };
};

#endif // SPTCPSERVER_H

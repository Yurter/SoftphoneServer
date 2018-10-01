#ifndef SPTHREADSERVER_H
#define SPTHREADSERVER_H

#include <QThread>
#include "SpTcpServer.h"
#include "SpUdpServer.h"

class SpThreadServer : public QThread
{
    Q_OBJECT
public:
    explicit SpThreadServer(QObject *parent = nullptr);
    ~SpThreadServer();

    void                setPort(quint16 port);
    void                setDatabasePath(QString path);

signals:

public slots:

protected:
    void                run();

private:
    SpTcpServer *tcpServer;
    SpUdpServer *udpServer;

    QString             mDatabasePath;
    quint16             mPort;
};

#endif // SPTHREADSERVER_H

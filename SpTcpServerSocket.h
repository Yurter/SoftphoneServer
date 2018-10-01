#ifndef SPTCPSERVERSOCKET_H
#define SPTCPSERVERSOCKET_H

#include "SpThreadTcpSocket.h"
#include "Logger.h"

class SpTcpServerSocket : public SpThreadTcpSocket
{
    Q_OBJECT
public:
    explicit SpTcpServerSocket(qintptr descriptor, QObject *parent = nullptr);
    virtual ~SpTcpServerSocket();

public slots:
    void            onReadyRead();
    void            onReadyToResponse(QByteArray data);

protected:
    void            run();

protected:
    qintptr         mSocketDescriptor;
};

#endif // SPTCPSERVERSOCKET_H

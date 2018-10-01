#include "SpThreadTcpSocket.h"

SpThreadTcpSocket::SpThreadTcpSocket(QObject *parent) :
    QThread(parent)
{

}

SpThreadTcpSocket::~SpThreadTcpSocket()
{
    delete mTcpSocket;
}

void SpThreadTcpSocket::onDisconnected()
{
    quit();
}

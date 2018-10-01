#include "SpThreadUdpSocket.h"

SpThreadUdpSocket::SpThreadUdpSocket(QHostAddress address, quint16 port, QObject *parent) :
    QThread(parent),
    mAddress(address),
    mPort(port)
{

}

SpThreadUdpSocket::SpThreadUdpSocket(qintptr descriptor, QObject *parent)
{

}

SpThreadUdpSocket::~SpThreadUdpSocket()
{
    delete mUdpSocket;
}

void SpThreadUdpSocket::run()
{
    mUdpSocket = new QUdpSocket(this);
    mUdpSocket->bind(mAddress, mPort);
    connect(mUdpSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));

    exec();
}

void SpThreadUdpSocket::sendDatagram(QNetworkDatagram datagram)
{
    if (this->isRunning()) {
        mUdpSocket->writeDatagram(datagram);
    }
}

void SpThreadUdpSocket::readPendingDatagrams()
{
    while (mUdpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = mUdpSocket->receiveDatagram();
        emit processTheDatagram(datagram);
    }
}

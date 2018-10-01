#include "SpUdpServer.h"

SpUdpServer::SpUdpServer(QObject *parent) : QObject(parent)
{

}

SpUdpServer::~SpUdpServer()
{

}

void SpUdpServer::start()
{
    if (mDatabase.openDatabase()) {
        if (listen(QHostAddress::Any, mPort)) {
            qInfo() << "UDP Server listen address:" << QHostAddress::Any;
            qInfo() << "UDP Server listen port:" << mPort;
            qInfo() << "UDP Server is running.";
        } else {
            qInfo() << "UDP Server could not start: Could not start listen incoming connection.";
            QCoreApplication::exit(ListenError);
        }
    } else {
        qInfo() << "UDP Server could not start: Could not open database.";
        QCoreApplication::exit(DatabaseError);
    }
}

void SpUdpServer::incomingConnection(qintptr handle)
{
    SpThreadUdpSocket *socket = new SpThreadUdpSocket(handle);
    connect(socket, SIGNAL(finished()), socket, SLOT(deleteLater()));
    connect(socket, SIGNAL(socketGotRequest(QByteArray)), this, SLOT(onSocketGotRequest(QByteArray)), Qt::DirectConnection);
    connect(this, SIGNAL(responseIsReady(QByteArray)), socket, SLOT(onResponseIsReady(QByteArray)));
    socket->start();
}

bool SpUdpServer::listen(QHostAddress address, quint16 port)
{
    return false;
}

void SpUdpServer::onDatagramReceived(QNetworkDatagram datagram)
{

}

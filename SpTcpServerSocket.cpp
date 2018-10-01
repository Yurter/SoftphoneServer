#include "SpTcpServerSocket.h"

SpTcpServerSocket::SpTcpServerSocket(qintptr descriptor, QObject *parent) :
    SpThreadTcpSocket(parent),
    mSocketDescriptor(descriptor)
{
    DEBUG_LOG("Создан объект класса SpTcpServerSocket.");
}

SpTcpServerSocket::~SpTcpServerSocket()
{
    DEBUG_LOG("Уничтожен объект класса SpTcpServerSocket.");
    quit();
}

void SpTcpServerSocket::onReadyRead()
{
    QByteArray request;
    do {
        request.append(mTcpSocket->readAll());
    } while (mTcpSocket->waitForReadyRead(100));

    DEBUG_LOG("[SpTcpServerSocket] Принято " + QString::number(request.size()) + " байт.");
    emit processTheData(request);
    while (mData.isEmpty()) { msleep(1); }

    qint64 count = mTcpSocket->write(mData);
    DEBUG_LOG("[SpTcpServerSocket] В ответ записано " + QString::number(count) + " байт.");
    mTcpSocket->waitForBytesWritten();
    mTcpSocket->close();
    DEBUG_LOG("[SpTcpServerSocket] Соединение разорвано.");
}

void SpTcpServerSocket::onReadyToResponse(QByteArray data)
{
    DEBUG_LOG("[SpTcpServerSocket] Для отправки подготовлено " + QString::number(data.size()) + " байт.");
    mData = data;
}

void SpTcpServerSocket::run()
{
    DEBUG_LOG("[SpTcpServerSocket] Сокет запущен.");
    mTcpSocket = new QTcpSocket();
    mTcpSocket->setSocketDescriptor(mSocketDescriptor);
    connect(mTcpSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()), Qt::DirectConnection);
    connect(mTcpSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    exec();
}

#include "SpThreadServer.h"

SpThreadServer::SpThreadServer(QObject *parent) :
    QThread(parent)
{
    DEBUG_LOG("Создан объект класса SpThreadServer.");
}

SpThreadServer::~SpThreadServer()
{
    DEBUG_LOG("Уничтожен объект класса SpThreadServer.");
    Logger::getInstance().closeLogFile();
    tcpServer->deleteLater();
    udpServer->deleteLater();
}

void SpThreadServer::setPort(quint16 port)
{
    DEBUG_LOG("[SpThreadServer] Установлен порт сервера: " + QString::number(port));
    mPort = port;
}

void SpThreadServer::setDatabasePath(QString path)
{
    DEBUG_LOG("[SpThreadServer] Установлено имя файла базы данных: " + path);
    mDatabasePath = path;
}

void SpThreadServer::run()
{
    DEBUG_LOG("[SpThreadServer] Потоковый сервер заупщен.");
    DEBUG_LOG("[SpThreadServer] Запуск TCP сервера.");

    tcpServer = new SpTcpServer();
    tcpServer->setDatabasePath(mDatabasePath);
    tcpServer->setPort(mPort);
    tcpServer->start();

    //DEBUG_LOG("[SpThreadServer] Запуск UDP сервера.");
    //udpServer = new SpUdpServer();

    exec();
}

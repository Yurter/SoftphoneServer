#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include "Logger.h"
#include "SpThreadServer.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.addLibraryPath(app.applicationDirPath() + "/lib");

    QCommandLineParser parser;
    QList<QCommandLineOption> optionList;
    optionList.append({{"d", "debug"}, "Log debug messages."});
    optionList.append({{"c", "console"}, "Output log to the console."});
    optionList.append({{"p", "port"}, "Application connection port.", "port", "1067"});
    optionList.append({"database", "The name of the database file.", "database", "softphoneDatabase.db3"});
    parser.addOptions(optionList);
    parser.process(app);

    bool debugOutput = parser.isSet("debug");
    Logger::getInstance().enableDebugOutput(debugOutput);

    bool consoleOutput = parser.isSet("console");
    Logger::getInstance().enableConsoleOutput(consoleOutput);

    SpThreadServer *server = new SpThreadServer();
    server->setPort(quint16(parser.value("port").toInt()));
    server->setDatabasePath(parser.value("database"));
    server->start();

    app.exec();
    server->deleteLater();
}

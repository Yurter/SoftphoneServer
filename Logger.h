#ifndef LOGGER_H
#define LOGGER_H

#include <QFile>
#include <QDebug>
#include <QDateTime>

#define DEFAULT_FILE_NAME "log"
#define CODE_POSITION __FILE__ + __LINE__
#define VARIABLE(x) (#x) + QString(" is ") + (x) + "; "
#define watch(x) qInfo() << (#x) << "is" << (x) << endl
#define DEBUG_LOG(log) Logger::getInstance().print(Logger::Debug, QString(log))
#define ERROR_LOG(log) Logger::getInstance().print(Logger::Error, QString(log))
#define INFO_LOG(log) Logger::getInstance().print(Logger::Info, QString(log))


class Logger;

class LoggerDestroyer
{
private:
    Logger* p_instance;

public:
    ~LoggerDestroyer();
    void initialize(Logger* p);
};

class Logger : public QObject
{
    Q_OBJECT
private:
    static Logger              *p_instance;
    static LoggerDestroyer      destroyer;

protected:
    Logger();
    Logger(const Logger&);
    Logger& operator=(Logger&);
    ~Logger();
    friend class LoggerDestroyer;

public:
    enum LogType {
        Debug = 1,
        Info  = 2,
        Error = 4
    };
    static Logger& getInstance();

    void    setLogFile(QString name);
    void    closeLogFile();
    void    print(LogType type, QString data);
    void    setLogMode(int mode);
    void    enableConsoleOutput(bool enable);
    void    enableDebugOutput(bool enable);

private:
    void    openFile(QString name);
    void    closeFile();

private:
    QFile   mFile;
    int     mLogMode;
    bool    mConsoleOutputEnabele;
};

#endif // LOGGER_H

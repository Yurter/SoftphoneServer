#include "Logger.h"

Logger *Logger::p_instance = nullptr;
LoggerDestroyer Logger::destroyer;

LoggerDestroyer::~LoggerDestroyer()
{
    p_instance->deleteLater();
}

void LoggerDestroyer::initialize(Logger* p)
{
    p_instance = p;
}

Logger::Logger()
{
    openFile(DEFAULT_FILE_NAME);
    mLogMode = Info|Error;
    mConsoleOutputEnabele = false;
}

Logger::Logger(const Logger &)
{

}

Logger &Logger::operator=(Logger &)
{

}

Logger::~Logger()
{
    closeFile();
}

Logger& Logger::getInstance()
{
    if(!p_instance) {
        p_instance = new Logger();
        destroyer.initialize(p_instance);
    }
    return *p_instance;
}

void Logger::setLogFile(QString name)
{
    openFile(name);
}

void Logger::closeLogFile()
{
    closeFile();
}

void Logger::print(LogType type, QString data)
{
    if (!(mLogMode & type))
        return;

    QString prefix;
    switch (type) {
    case Debug:
        prefix = "[Debug] ";
        break;
    case Info:
        prefix = "[Info]  ";
        break;
    case Error:
        prefix = "[ERROR] ";
        break;
    }

    prefix.append(QTime::currentTime().toString());
    QString log = prefix + " " + data;
    mFile.write(log.toUtf8() + '\n');

    if (mConsoleOutputEnabele)
        qInfo().noquote() << log;
}

void Logger::enableConsoleOutput(bool enable)
{
    mConsoleOutputEnabele = enable;
}

void Logger::enableDebugOutput(bool enable)
{
    if (enable)
        mLogMode = Debug|Info|Error;
    else
        mLogMode = Info|Error;
}

void Logger::openFile(QString name)
{
    closeFile();

    mFile.setFileName(name);
    if (!mFile.open(QIODevice::WriteOnly | QIODevice::Append))
        qInfo() << "[Logger] Could not open file.";
    else
        mFile.write("Log started at: " + QDateTime::currentDateTime().toString().toUtf8() + '\n');
}

void Logger::closeFile()
{
    if (mFile.isOpen()) {
        mFile.write("Log ended at: " + QDateTime::currentDateTime().toString().toUtf8());
        mFile.write("\n\n\n");
        mFile.close();
    }
}

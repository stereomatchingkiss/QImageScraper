#include "mainwindow.hpp"

#include <QsLog.h>
#include <QsLogDest.h>

#include <QApplication>
#include <QDir>

void log_function(const QsLogging::LogMessage &message);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("ThamSoft");
    QCoreApplication::setOrganizationDomain("https://github.com/stereomatchingkiss");
    QCoreApplication::setApplicationName("QImageScraper");

    using namespace QsLogging;
    // 1. init the logging mechanism
    Logger& logger = Logger::instance();
    logger.setLoggingLevel(QsLogging::TraceLevel);
    const QString sLogPath(QDir(a.applicationDirPath()).filePath("log.txt"));

    // 2. add two destinations
    DestinationPtrU fileDestination(DestinationFactory::MakeFileDestination(
                                        sLogPath, LogRotationOption::EnableLogRotation, MaxSizeBytes(512), MaxOldLogCount(2)));
    DestinationPtrU debugDestination(DestinationFactory::MakeDebugOutputDestination());
    DestinationPtrU functorDestination(DestinationFactory::MakeFunctorDestination(&log_function));
    logger.addDestination(std::move(debugDestination));
    logger.addDestination(std::move(fileDestination));
    logger.addDestination(std::move(functorDestination));

    MainWindow w;
    w.show();

    return a.exec();
}

void log_function(const QsLogging::LogMessage &message)
{
    qDebug() << "From log function: " << qPrintable(message.formatted)
             << " " << static_cast<int>(message.level);
}

#include "mainwindow.hpp"

#include <QsLog.h>
#include <QsLogDest.h>

#include <QApplication>
#include <QDir>

void log_function(const QsLogging::LogMessage &message);

void setup_logger(QApplication &a);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("ThamSoft");
    QCoreApplication::setOrganizationDomain("https://github.com/stereomatchingkiss");
    QCoreApplication::setApplicationName("QImageScraper");

    setup_logger(a);

    MainWindow w;
    w.show();

    return a.exec();
}

void log_function(const QsLogging::LogMessage &message)
{
    qDebug() << "From log function: " << qPrintable(message.formatted)
             << " " << static_cast<int>(message.level);
}

void setup_logger(QApplication &app)
{
    using namespace QsLogging;

    Logger& logger = Logger::instance();
    logger.setLoggingLevel(QsLogging::TraceLevel);
    const QString sLogPath(QDir(app.applicationDirPath()).filePath("log.txt"));

    // 2. add two destinations
    DestinationPtr fileDestination(DestinationFactory::MakeFileDestination(
                                       sLogPath, EnableLogRotation,
                                       MaxSizeBytes(1024*1024*10), MaxOldLogCount(2)));
    DestinationPtr debugDestination(DestinationFactory::MakeDebugOutputDestination());
    DestinationPtr functorDestination(DestinationFactory::MakeFunctorDestination(&log_function));
    logger.addDestination(debugDestination);
    logger.addDestination(fileDestination);
    logger.addDestination(functorDestination);
}

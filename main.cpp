#include "mainwindow.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("ThamSoft");
    QCoreApplication::setOrganizationDomain("https://github.com/stereomatchingkiss");
    QCoreApplication::setApplicationName("QImageScraper");

    MainWindow w;
    w.show();

    return a.exec();
}

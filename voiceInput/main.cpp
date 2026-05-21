#include "mainwindow.h"


#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("NotedioProject");
    QCoreApplication::setApplicationName("notedio");
    MainWindow w;
    w.show();
    return QCoreApplication::exec();
}

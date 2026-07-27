#include "ui/MainWindow.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qDebug() << "[main] starting notedio";

    MainWindow window;
    window.show();

    return QApplication::exec();
}

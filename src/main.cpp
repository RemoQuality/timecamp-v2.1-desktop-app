#include "MainWidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationDomain("TimeCamp.com");
    QCoreApplication::setOrganizationName("Time Solutions");
    QCoreApplication::setApplicationName("TimeCamp tracking tool");
    QCoreApplication::setApplicationVersion("2.0.0.0");

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);
    MainWidget w;

    w.setWindowTitle("TimeCamp");

    return app.exec();
}

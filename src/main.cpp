#include <QApplication>
#include <QTimer>
#include <QObject>

#include "Settings.h"
#include "MainWidget.h"
#include "Comms.h"

int main(int argc, char *argv[])
{
    // Caches are saved in %localappdata%/org_name/app_name
    // Eg. C:\Users\timecamp\AppData\Local\Time Solutions\TimeCamp Desktop
    // Settings are saved in registry: HKEY_CURRENT_USER\Software\Time Solutions\TimeCamp Desktop

    QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
    QCoreApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);
    QCoreApplication::setApplicationName(APPLICATION_NAME);
    QCoreApplication::setApplicationVersion(APPLICATION_VERSION);

    // Enable high dpi support
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);

    // create main widget
    MainWidget w;
    w.setWindowTitle(WINDOW_NAME);

    // send updates from DB to server
    QTimer *timer = new QTimer();
    QObject::connect(timer, SIGNAL(timeout()), &Comms::instance(), SLOT(timedUpdates()));
    timer->start(30*1000); // do it every 30s

    return app.exec();
}

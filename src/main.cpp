#include <QApplication>
#include <QTimer>
#include <QObject>

#include "Settings.h"
#include "MainWidget.h"
#include "Comms.h"
#include "WindowEventsManager.h"

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

    // standard Qt init
    QApplication app(argc, argv);


    // create events manager
    WindowEventsManager *wem = new WindowEventsManager();

    // create main widget
    MainWidget w;
    //QObject::connect(&w, SIGNAL(pcActivitiesValueChanged(bool)), wem, SLOT(startOrStopThread(bool))); // Qt4
    QObject::connect(&w, &MainWidget::pcActivitiesValueChanged, wem, &WindowEventsManager::startOrStopThread); // Qt5
    w.init();


    // send updates from DB to server
    Comms *comms = new Comms();
    QTimer *syncDBtimer = new QTimer();
    //QObject::connect(timer, SIGNAL(timeout()), &Comms::instance(), SLOT(timedUpdates())); // Qt4
    QObject::connect(syncDBtimer, &QTimer::timeout, comms, &Comms::timedUpdates); // Qt5
    syncDBtimer->start(30*1000); // do it every 30s


    // 2 sec timer for updating submenu and other features
    QTimer *twoSecondTimer = new QTimer();
    //QObject::connect(twoSecondTimer, SIGNAL(timeout()), &w, SLOT(twoSecTimerTimeout())); // Qt4
    QObject::connect(twoSecondTimer, &QTimer::timeout, &w, &MainWidget::twoSecTimerTimeout); // Qt5
    twoSecondTimer->start(2 * 1000);

    return app.exec();
}

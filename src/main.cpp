#include <QApplication>
#include <QTimer>

#include "Settings.h"
#include "MainWidget.h"
#include "Comms.h"
#include "WindowEvents.h"
#include "WindowEventsManager.h"
#include "TrayManager.h"

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

    QIcon appIcon = QIcon(MAIN_ICON);
//    appIcon.addFile(":/Icons/res/AppIcon32.png");
//    appIcon.addFile(":/Icons/res/AppIcon128.png");
    app.setWindowIcon(appIcon);


    // create events manager
    auto *windowEventsManager = new WindowEventsManager();

    // create main widget
    MainWidget mainWidget;
    mainWidget.init();

    // create tray manager
    auto *trayManager = new TrayManager();
    QObject::connect(&mainWidget, &MainWidget::pageStatusChanged, trayManager, &TrayManager::loginLogout);
    QObject::connect(&mainWidget, &MainWidget::timerStatusChanged, trayManager, &TrayManager::updateStopMenu);
    QObject::connect(trayManager, &TrayManager::pcActivitiesValueChanged, windowEventsManager, &WindowEventsManager::startOrStopThread);
    trayManager->setupTray(&mainWidget);

    // send updates from DB to server
    auto *comms = new Comms();
    auto *syncDBtimer = new QTimer();
    //QObject::connect(timer, SIGNAL(timeout()), &Comms::instance(), SLOT(timedUpdates())); // Qt4
    QObject::connect(syncDBtimer, &QTimer::timeout, comms, &Comms::timedUpdates); // Qt5
    syncDBtimer->start(30 * 1000); // sync DB every 30s


    // 2 sec timer for updating submenu and other features
    auto *twoSecondTimer = new QTimer();
    //QObject::connect(twoSecondTimer, SIGNAL(timeout()), &mainWidget, SLOT(twoSecTimerTimeout())); // Qt4
    QObject::connect(twoSecondTimer, &QTimer::timeout, &mainWidget, &MainWidget::twoSecTimerTimeout); // Qt5
    // above timeout triggers func that emits checkIsIdle when logged in
    QObject::connect(&mainWidget, &MainWidget::checkIsIdle, windowEventsManager->getCaptureEventsThread(), &WindowEvents::checkIdleStatus); // Qt5
    twoSecondTimer->start(2 * 1000);

    return app.exec();
}

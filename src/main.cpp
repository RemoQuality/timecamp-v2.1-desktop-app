#include <QApplication>
#include <QTimer>
#include <QStandardPaths>
#include <QLibraryInfo>

#ifdef Q_OS_MACOS

#include "Utils_M.h"

#endif

#include "Settings.h"
#include "Autorun.h"
#include "MainWidget.h"
#include "Comms.h"
#include "DbManager.h"
#include "AutoTracking.h"
#include "TrayManager.h"
#include "DataCollector/WindowEvents.h"
#include "WindowEventsManager.h"
#include "Widget/FloatingWidget.h"

#include "third-party/vendor/de/skycoder42/qhotkey/QHotkey/qhotkey.h"
#include "third-party/QTLogRotation/logutils.h"

void firstRun()
{
    QSettings settings;

    if (settings.value(SETT_IS_FIRST_RUN, true).toBool()) {
        Autorun::enableAutorun();
#ifdef Q_OS_MACOS
        enableAssistiveDevices();
#endif
    }
    settings.setValue(SETT_IS_FIRST_RUN, false);
}

int main(int argc, char *argv[])
{

    // Caches are saved in %localappdata%/org_name/APPLICATION_NAME
    // Eg. C:\Users\timecamp\AppData\Local\Time Solutions\TimeCamp Desktop
    // Settings are saved in registry: HKEY_CURRENT_USER\Software\Time Solutions\TimeCamp Desktop

    QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
    QCoreApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);
    QCoreApplication::setApplicationName(APPLICATION_NAME);
    QCoreApplication::setApplicationVersion(APPLICATION_VERSION);

    // install log handler
    LOGUTILS::initLogging();

    // Enable high dpi support
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    // OpenGL is a mess; lets just use software and hammer the CPU
    // https://wiki.qt.io/QtWebEngine/Rendering
    // http://lists.qt-project.org/pipermail/qtwebengine/2017-August/000462.html
    // https://forum.qt.io/topic/82530/qt5-can-webgl-work-with-angle-on-windows-via-qtwebengine
    // https://forum.qt.io/topic/51257/imx6-qtwebengine-black-surfaces/9

    QCoreApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);

    // prevent our app from closing
    QGuiApplication::setQuitOnLastWindowClosed(false);

    // standard Qt init
    QApplication app(argc, argv);

    // debugging library locations (most useful for Linux debugging)
    for(int i = 0; i < 13; i++) {
        qInfo() << "Location " << i << QLibraryInfo::location(QLibraryInfo::LibraryLocation(i));
    }
    qInfo() << "Loc: " << QCoreApplication::applicationDirPath() << '\n';
    qInfo() << "qt.conf " << QDir(QCoreApplication::applicationDirPath()).exists("qt.conf") << '\n';

    firstRun();

    QIcon appIcon = QIcon(MAIN_ICON);
    appIcon.addFile(":/Icons/AppIcon_16.png");
    appIcon.addFile(":/Icons/AppIcon_32.png");
    appIcon.addFile(":/Icons/AppIcon_48.png");
    appIcon.addFile(":/Icons/AppIcon_64.png");
    appIcon.addFile(":/Icons/AppIcon_128.png");
    appIcon.addFile(":/Icons/AppIcon_256.png");
    QApplication::setWindowIcon(appIcon);

    // create DB Manager instance early, as it needs some time to prepare queries etc
    DbManager *dbManager = &DbManager::instance();
    AutoTracking *autoTracking = &AutoTracking::instance();

    // create events manager
    WindowEventsManager *windowEventsManager = &WindowEventsManager::instance();

    // create main widget
    MainWidget mainWidget;
    mainWidget.setWindowIcon(appIcon);

    // create tray manager
    TrayManager *trayManager = &TrayManager::instance();
    QObject::connect(&mainWidget, &MainWidget::pageStatusChanged, trayManager, &TrayManager::loginLogout);
    QObject::connect(&mainWidget, &MainWidget::timerStatusChanged, trayManager, &TrayManager::updateStopMenu);
    QObject::connect(&mainWidget, &MainWidget::lastTasksChanged, trayManager, &TrayManager::updateRecentTasks);
    QObject::connect(trayManager, &TrayManager::taskSelected, &mainWidget, &MainWidget::startTaskByID);
    QObject::connect(trayManager, &TrayManager::pcActivitiesValueChanged, windowEventsManager, &WindowEventsManager::startOrStopThread);

    // send updates from DB to server
    Comms *comms = &Comms::instance();
    auto *syncDBtimer = new QTimer();
    //QObject::connect(timer, SIGNAL(timeout()), &Comms::instance(), SLOT(timedUpdates())); // Qt4
    QObject::connect(syncDBtimer, &QTimer::timeout, comms, &Comms::timedUpdates); // Qt5

    // Away time bindings
    QObject::connect(windowEventsManager, &WindowEventsManager::updateAfterAwayTime, comms, &Comms::timedUpdates);
    QObject::connect(windowEventsManager, &WindowEventsManager::openAwayTimeManagement, &mainWidget, &MainWidget::goToAwayPage);

    // Stopped logging bind
    QObject::connect(windowEventsManager, &WindowEventsManager::dataCollectingStopped, comms, &Comms::clearLastApp);

    // Save apps to sqlite on signal-slot basis
    QObject::connect(comms, &Comms::DbSaveApp, dbManager, &DbManager::saveAppToDb);
    QObject::connect(comms, &Comms::DbSaveApp, autoTracking, &AutoTracking::checkAppKeywords);
    QObject::connect(autoTracking, &AutoTracking::foundTask, &mainWidget, &MainWidget::startTaskByTaskObj);
    QObject::connect(&mainWidget, &MainWidget::startTaskViaObjToID, &mainWidget, &MainWidget::startTaskByID);


    // 2 sec timer for updating submenu and other features
    auto *twoSecondTimer = new QTimer();
    //QObject::connect(twoSecondTimer, SIGNAL(timeout()), &mainWidget, SLOT(twoSecTimerTimeout())); // Qt4
    QObject::connect(twoSecondTimer, &QTimer::timeout, &mainWidget, &MainWidget::twoSecTimerTimeout); // Qt5
    // above timeout triggers func that emits checkIsIdle when logged in
    QObject::connect(&mainWidget, &MainWidget::checkIsIdle, windowEventsManager->getCaptureEventsThread(), &WindowEvents::checkIdleStatus); // Qt5


    auto hotkeyNewTimer = new QHotkey(QKeySequence(KB_SHORTCUTS_START_TIMER), true, &app);
    QObject::connect(hotkeyNewTimer, &QHotkey::activated, &mainWidget, &MainWidget::startTask);

    auto hotkeyStopTimer = new QHotkey(QKeySequence(KB_SHORTCUTS_STOP_TIMER), true, &app);
    QObject::connect(hotkeyStopTimer, &QHotkey::activated, &mainWidget, &MainWidget::stopTask);

    auto hotkeyOpenWindow = new QHotkey(QKeySequence(KB_SHORTCUTS_OPEN_WINDOW), true, &app);
    QObject::connect(hotkeyOpenWindow, &QHotkey::activated, trayManager, &TrayManager::openCloseWindowAction);

    //
    QObject::connect(&mainWidget, &MainWidget::pageStatusChanged, [&syncDBtimer](bool loggedIn, QString title)
    {
        if (!loggedIn) {
            if(syncDBtimer->isActive()) {
                qInfo("Stopping DB Sync timer");
                syncDBtimer->stop();
            }
        } else {
            if(!syncDBtimer->isActive()) {
                qInfo("Restarting DB Sync timer");
                syncDBtimer->start();
            }
        }
    });

    // everything connected via QObject, now heavy lifting
    trayManager->setupTray(&mainWidget); // create tray
    auto *theWidget = new FloatingWidget(); // FloatingWidget can't be bound to mainwidget (it won't set state=visible when main is hidden)
    QObject::connect(&mainWidget, &MainWidget::timerStatusChanged, theWidget, &FloatingWidget::updateWidgetStatus);
    QObject::connect(theWidget, &FloatingWidget::taskNameClicked, &mainWidget, &MainWidget::startTask);
    QObject::connect(theWidget, &FloatingWidget::playButtonClicked, &mainWidget, &MainWidget::startTask);
    QObject::connect(theWidget, &FloatingWidget::pauseButtonClicked, &mainWidget, &MainWidget::stopTask);
    trayManager->setWidget(theWidget);
    trayManager->setupSettings();
    mainWidget.init(); // init the WebView
    comms->timedUpdates(); // fetch userInfo, userSettings, send apps since last update

    // now timers
    syncDBtimer->start(30 * 1000); // sync DB every 30s
    twoSecondTimer->start(2 * 1000);

    return QApplication::exec();
}

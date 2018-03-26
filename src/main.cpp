#include <QApplication>
#include <QTimer>
#include <QStandardPaths>
#include <ctime>
#include <iomanip>

#ifdef Q_OS_MACOS
#include "Utils_M.h"
#endif

#include "Settings.h"
#include "MainWidget.h"
#include "Comms.h"
#include "WindowEvents.h"
#include "WindowEventsManager.h"
#include "TrayManager.h"
#include "Autorun.h"

#include "vendor/de/skycoder42/qhotkey/QHotkey/qhotkey.h"


void firstRun() {
    QSettings settings;

    if(settings.value(SETT_IS_FIRST_RUN, true).toBool()) {
        Autorun::enableAutorun();
#ifdef Q_OS_MACOS
        enableAssistiveDevices();
#endif
    }
    settings.setValue(SETT_IS_FIRST_RUN, false);
}

void myMessageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg)
{
    std::time_t stdtime = std::time(nullptr);
//    std::cout << "UTC:       " << std::put_time(std::gmtime(&stdtime), "%H:%M:%S") << '\n';
//    std::cout << "local:     " << std::put_time(std::localtime(&stdtime), "%H:%M:%S") << '\n';
    char timestring[100];
    std::strftime(timestring, sizeof(timestring), "%H:%M:%S", std::gmtime(&stdtime)); // UTC, localtime for local

    QString txt;
    txt += "[";
    txt += timestring;
    txt += "] ";
    switch (type) {
        case QtInfoMsg:
            txt += QString("Info:\t%1").arg(msg);
            break;
        case QtDebugMsg:
            txt += QString("Debug:\t%1").arg(msg);
            break;
        case QtWarningMsg:
            txt += QString("Warning:\t%1").arg(msg);
            break;
        case QtCriticalMsg:
            txt += QString("Critical:\t%1").arg(msg);
            break;
        case QtFatalMsg:
            txt += QString("Fatal:\t%1").arg(msg);
            break;
    }
    QFile outFile(QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation).first() + "/" + LOG_FILENAME);
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << endl;
}

int main(int argc, char *argv[])
{
    // install log handler
    qInstallMessageHandler(myMessageHandler);

    // Caches are saved in %localappdata%/org_name/APPLICATION_NAME
    // Eg. C:\Users\timecamp\AppData\Local\Time Solutions\TimeCamp Desktop
    // Settings are saved in registry: HKEY_CURRENT_USER\Software\Time Solutions\TimeCamp Desktop

    QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
    QCoreApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);
    QCoreApplication::setApplicationName(APPLICATION_NAME);
    QCoreApplication::setApplicationVersion(APPLICATION_VERSION);

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
    firstRun();

    QIcon appIcon = QIcon(MAIN_ICON);
//    appIcon.addFile(":/Icons/res/AppIcon32.png");
//    appIcon.addFile(":/Icons/res/AppIcon128.png");
    app.setWindowIcon(appIcon);


    // create events manager
    auto *windowEventsManager = new WindowEventsManager();

    // create main widget
    MainWidget mainWidget;

    // create tray manager
    auto *trayManager = new TrayManager();
    QObject::connect(&mainWidget, &MainWidget::pageStatusChanged, trayManager, &TrayManager::loginLogout);
    QObject::connect(&mainWidget, &MainWidget::timerStatusChanged, trayManager, &TrayManager::updateStopMenu);
    QObject::connect(&mainWidget, &MainWidget::windowStatusChanged, trayManager, &TrayManager::openCloseWindowText);
    QObject::connect(trayManager, &TrayManager::pcActivitiesValueChanged, windowEventsManager, &WindowEventsManager::startOrStopThread);

    // send updates from DB to server
    auto *comms = new Comms();
    auto *syncDBtimer = new QTimer();
    //QObject::connect(timer, SIGNAL(timeout()), &Comms::instance(), SLOT(timedUpdates())); // Qt4
    QObject::connect(syncDBtimer, &QTimer::timeout, comms, &Comms::timedUpdates); // Qt5

    // Away time bindings
    QObject::connect(windowEventsManager, &WindowEventsManager::updateAfterAwayTime, comms, &Comms::timedUpdates);
    QObject::connect(windowEventsManager, &WindowEventsManager::openAwayTimeManagement, &mainWidget, &MainWidget::goToAwayPage);


    // 2 sec timer for updating submenu and other features
    auto *twoSecondTimer = new QTimer();
    //QObject::connect(twoSecondTimer, SIGNAL(timeout()), &mainWidget, SLOT(twoSecTimerTimeout())); // Qt4
    QObject::connect(twoSecondTimer, &QTimer::timeout, &mainWidget, &MainWidget::twoSecTimerTimeout); // Qt5
    // above timeout triggers func that emits checkIsIdle when logged in
    QObject::connect(&mainWidget, &MainWidget::checkIsIdle, windowEventsManager->getCaptureEventsThread(), &WindowEvents::checkIdleStatus); // Qt5


    auto hotkeyNewTimer = new QHotkey(QKeySequence("ctrl+alt+N"), true, &app);
    QObject::connect(hotkeyNewTimer, &QHotkey::activated, &mainWidget, &MainWidget::startTask);

    auto hotkeyStopTimer = new QHotkey(QKeySequence("ctrl+alt+M"), true, &app);
    QObject::connect(hotkeyStopTimer, &QHotkey::activated, &mainWidget, &MainWidget::stopTask);

    auto hotkeyOpenWindow = new QHotkey(QKeySequence("ctrl+alt+/"), true, &app);
    QObject::connect(hotkeyOpenWindow, &QHotkey::activated, trayManager, &TrayManager::openCloseWindowAction);

    // everything connected via QObject, now heavy lifting
    trayManager->setupTray(&mainWidget); // create tray
    mainWidget.init(); // init the WebView
    Comms::instance().timedUpdates(); // fetch userInfo, userSettings, send apps since last update

    // now timers
    syncDBtimer->start(30 * 1000); // sync DB every 30s
    twoSecondTimer->start(2 * 1000);

    return app.exec();
}

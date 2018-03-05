#include "MainWidget.h"
#include "ui_MainWidget.h"

#include "Settings.h"

#ifdef __linux__
#include "WindowEvents_U.h"
WindowEvents *captureEventsThread = new WindowEvents_U();
#elif _WIN32
#include "WindowEvents_W.h"
#include "Autorun.h"

WindowEvents *captureEventsThread = new WindowEvents_W();
#else
    // mac
#endif

void startThread(QThread *thread)
{
    thread->start();
}

void stopThread(QThread *thread)
{
    thread->requestInterruption(); // if it checks for isInterruptionRequested
    if(thread->isRunning()){
        thread->exit(); // if it uses QEventLoop
        if(thread->isRunning()){
            thread->terminate(); // force close
        }
    }
}

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    // set some defaults
    loggedIn = false;
    timerIsRunning = false;
    apiKey = "";
    timerName = "";

    restoreGeometry(settings.value("mainWindowGeometry").toByteArray()); // from QWidget; restore saved window position

    this->setupWebview(); // starts the embedded webpage
    this->setupTray(parent); // creates all actions in tray here
    this->setupSettings(); // sets tray values
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::resizeEvent(QResizeEvent *event)
{
    this->setUpdatesEnabled(false);
    QTWEView->resize(size()); // resize webview
    this->setUpdatesEnabled(true);
    QWidget::resizeEvent(event); // do the default "whatever happens on resize"
}

void MainWidget::closeEvent(QCloseEvent *event)
{
    settings.setValue("mainWindowGeometry", saveGeometry()); // save window position
    hide(); // hide our window when X was pressed
    event->ignore(); // don't do the default action (which usually is app exit)
}

void MainWidget::twoSecTimerTimeout()
{
    if(loggedIn) {
        fetchAPIkey();
        checkIsTimerRunning();
    }
}

void MainWidget::setupSettings()
{
    // set checkboxes
    autoStartAct->setChecked(settings.value(SETT_AUTOSTART, false).toBool());
    trackerAct->setChecked(settings.value(SETT_TRACK_PC_ACTIVITIES, false).toBool());

    // act on the saved settings
    this->autoStart(autoStartAct->isChecked());
    this->tracker(trackerAct->isChecked());
}

void MainWidget::setupWebview()
{
    QTWEView = new QWebEngineView(this);
    QTWEView->setContextMenuPolicy(Qt::NoContextMenu); // disable context menu in embedded webpage

    QTWEProfile = new QWebEngineProfile(QCoreApplication::applicationName(), QTWEView); // set "profile" as appName
    QTWEProfile->setHttpUserAgent(CONN_USER_AGENT); // add useragent to this profile

    QTWESettings = QTWEProfile->settings();
    QTWESettings->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true); // modify settings: enable Fullscreen

    TCri = new TCRequestInterceptor();
    QTWEProfile->setRequestInterceptor(TCri);

    QTWEPage = new QWebEnginePage(QTWEProfile, QTWEView);
    QTWEView->setPage(QTWEPage);

//    pagePointer = m_pWebEngineView->page();

    QTWEPage->load(QUrl("https://www.timecamp.com/dashboard"));
//    QTWEPage->load(QUrl("http://request.urih.com/"));

    refreshBind = new QShortcut(QKeySequence::Refresh, this);
    refreshBind->setContext(Qt::ApplicationShortcut);
    connect(refreshBind, &QShortcut::activated, this, &MainWidget::webviewRefresh);

    fullscreenBind = new QShortcut(QKeySequence::FullScreen, this);
    fullscreenBind->setContext(Qt::ApplicationShortcut);
    connect(fullscreenBind, &QShortcut::activated, this, &MainWidget::webviewFullscreen);


    connect(QTWEPage, &QWebEnginePage::titleChanged, this, &MainWidget::webpageTitleChanged);
}

void MainWidget::webpageTitleChanged(QString title)
{
//    qInfo("[NEW_TC]: Webpage title changed: ");
//    qInfo(title.toLatin1().constData());
    trayIcon->setToolTip(title);
    if(!loggedIn){
        checkIfLoggedIn(title);
    }
}

void MainWidget::webviewRefresh()
{
    qDebug("[NEW_TC]: page refresh");
    this->setUpdatesEnabled(false);
    QTWEPage->triggerAction(QWebEnginePage::ReloadAndBypassCache);
    this->setUpdatesEnabled(true);
}

void MainWidget::webviewFullscreen()
{
    qDebug("[NEW_TC]: go full screen");
    this->setUpdatesEnabled(false);
    this->setWindowState(this->windowState() ^ Qt::WindowFullScreen);
    this->setUpdatesEnabled(true);
}

void MainWidget::checkIfLoggedIn(QString title)
{
    if(title == "Timer Timesheet | TimeCamp"){
        loggedIn = true;
        twoSecTimerTimeout();
    }
}


void MainWidget::setupTray(QWidget *parent)
{
    if(QSystemTrayIcon::isSystemTrayAvailable() == false){
        QMessageBox::critical(this,":(","Ninja Mode is not available on this computer. Try again later :P");
    }

    trayMenu = new QMenu(parent);
    createActions(trayMenu);

    trayIcon = new QSystemTrayIcon(this);

    trayIcon->setIcon(this->windowIcon());
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();


    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}


void MainWidget::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason != QSystemTrayIcon::Context){
        this->open();
    }
}

void MainWidget::open()
{
    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
    show();
}

void MainWidget::runJSinPage(QString js)
{
    QTWEPage->runJavaScript(js);
}

void MainWidget::startTask()
{
    if(!this->isVisible()){
        this->show();
    }
    this->runJSinPage("$('#timer-task-picker').click();");
}

void MainWidget::stopTask()
{
    this->runJSinPage("$('.btn-timer').click();");
}

void MainWidget::checkIsTimerRunning()
{
    QTWEPage->runJavaScript("angular.element(document.body).injector().get('TimerService').timer.isTimerRunning",
    [this](const QVariant &v) {
//        qDebug() << "Timer running: " << v.toString();
        setIsTimerRunning(v.toBool());
    });
}

void MainWidget::fetchAPIkey()
{
    QString apiKey = "";
//    QTWEPage->runJavaScript("await window.apiService.getToken()",
    QTWEPage->runJavaScript("window.apiService.getToken().$$state.value",
    [this](const QVariant &v) {
//        qDebug() << "API Key: " << v.toString();
        setApiKey(v.toString());
    });
}

void MainWidget::fetchTimerName()
{
    QTWEPage->runJavaScript("TC.TimeTracking.getTask(angular.element(document.body).injector().get('TimerService').timer.task_id).name",
    [this](const QVariant &v) {
//        qDebug() << "Timer name: " << v.toString();
        setTimerName(v.toString());
    });
}

void MainWidget::autoStart(bool checked)
{
    settings.setValue(SETT_AUTOSTART, checked);
    if(checked){
        Autorun::enableAutorun();
    }else{
        Autorun::disableAutorun();
    }
}

void MainWidget::tracker(bool checked)
{
    settings.setValue(SETT_TRACK_PC_ACTIVITIES, checked);
    if(checked){
        startThread(captureEventsThread);
    } else {
        stopThread(captureEventsThread);
    }
}


void MainWidget::quit()
{
    QApplication::quit();
}

void MainWidget::createActions(QMenu *menu)
{
    openAct = new QAction(tr("Open"), this);
    openAct->setStatusTip(tr("Open browser"));
    connect(openAct, &QAction::triggered, this, &MainWidget::open);

    startTaskAct = new QAction(tr("Start timer"), this);
    startTaskAct->setStatusTip(tr("Go to task selection screen"));
    startTaskAct->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_N);
    startTaskAct->setShortcutVisibleInContextMenu(true);
    startTaskAct->setShortcutContext(Qt::ApplicationShortcut);
    connect(startTaskAct, &QAction::triggered, this, &MainWidget::startTask);

    stopTaskAct = new QAction(tr("Stop timer"), this);
    stopTaskAct->setStatusTip(tr("Stop currently running timer"));
    stopTaskAct->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_M);
    stopTaskAct->setShortcutVisibleInContextMenu(true);
    stopTaskAct->setShortcutContext(Qt::ApplicationShortcut);
    connect(stopTaskAct, &QAction::triggered, this, &MainWidget::stopTask);

    trackerAct = new QAction(tr("Track computer activities"), this);
    trackerAct->setCheckable(true);
    connect(trackerAct, &QAction::triggered, this, &MainWidget::tracker);

    autoStartAct = new QAction(tr("Launch app on login"), this);
    autoStartAct->setCheckable(true);
    connect(autoStartAct, &QAction::triggered, this, &MainWidget::autoStart);

    quitAct = new QAction(tr("Quit"), this);
    quitAct->setStatusTip(tr("Close the app"));
    connect(quitAct, &QAction::triggered, this, &MainWidget::quit);


    menu->addAction(openAct);
    menu->addSeparator();
    //menu->addAction(statusAct);
    menu->addAction(startTaskAct);
    menu->addAction(stopTaskAct);
    menu->addSeparator();
    menu->addAction(trackerAct);
    menu->addSeparator();
    menu->addAction(autoStartAct);
    menu->addSeparator();
    menu->addAction(quitAct);
}

void MainWidget::setApiKey(const QString &apiKey) {
    MainWidget::apiKey = apiKey;
    settings.setValue(SETT_APIKEY, apiKey); // save apikey to settings
}

void MainWidget::setTimerName(const QString &timerName) {
    MainWidget::timerName = timerName;
    stopTaskAct->setText("Stop " + timerName);
    stopTaskAct->setEnabled(true); // reenable task stopping
}

void MainWidget::setIsTimerRunning(bool isTimerRunning) {
    MainWidget::timerIsRunning = isTimerRunning;
    if(isTimerRunning){
        fetchTimerName(); // this will make menu say "Stop XYZ task"
    }else{ // no timer running
        setTimerName("timer"); // make the option say "Stop task"; hacky?
        stopTaskAct->setEnabled(false); // disable the option - gray it out
    }
}

const QString &MainWidget::getTimerName() const {
    return timerName;
}

bool MainWidget::isIsTimerRunning() const {
    return timerIsRunning;
}

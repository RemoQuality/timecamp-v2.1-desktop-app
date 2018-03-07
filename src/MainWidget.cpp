#include "MainWidget.h"
#include "ui_MainWidget.h"

#include "Settings.h"
#include "WindowEventsManager.h"


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

}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::init(){
    this->setWindowTitle(WINDOW_NAME);
    this->setupWebview(); // starts the embedded webpage
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
        emit checkIsIdle();
    }
}

void MainWidget::setupWebview()
{
    QTWEView = new QWebEngineView(this);
    QTWEView->setContextMenuPolicy(Qt::NoContextMenu); // disable context menu in embedded webpage

    QTWEProfile = new QWebEngineProfile(APPLICATION_NAME, QTWEView); // set "profile" as appName
    QTWEProfile->setHttpUserAgent(CONN_USER_AGENT); // add useragent to this profile

    QTWESettings = QTWEProfile->settings();
    QTWESettings->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true); // modify settings: enable Fullscreen

    TCri = new TCRequestInterceptor();
    QTWEProfile->setRequestInterceptor(TCri);

    QTWEPage = new QWebEnginePage(QTWEProfile, QTWEView);
    QTWEView->setPage(QTWEPage);

//    pagePointer = m_pWebEngineView->page();

    goToTimerPage(); // loads main app url
//    QTWEPage->load(QUrl(APPLICATION_URL));
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
    checkIfLoggedIn(title);
    emit pageStatusChanged(loggedIn, title);
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
    if (!title.toLower().contains(QRegExp("log in|login|register|create free account|create account|time tracking software"))) {
        loggedIn = true;
    } else {
        loggedIn = false; // when we log out, we need to set this variable again
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

bool MainWidget::checkIfOnTimerPage()
{
    if (QTWEPage->url().toString().indexOf("app#/timesheets/timer") != -1) {
        return true;
    }
    return false;
}

void MainWidget::goToTimerPage()
{
    if (!this->checkIfOnTimerPage()) {
        QEventLoop loop;
//        connect(QTWEPage, SIGNAL(loadFinished()), &loop, SLOT(quit()));
        connect(QTWEPage, &QWebEnginePage::loadFinished, &loop, &QEventLoop::quit);
        QTWEPage->load(QUrl(APPLICATION_URL));
        loop.exec();
    }
}

void MainWidget::startTask()
{
    goToTimerPage();
    this->stopTask(); // stop the last timer
    if(!this->isVisible()){
        this->show();
    }
    this->runJSinPage("if($('.btn-timer').text().trim().toLowerCase() == 'start timer') { $('.btn-timer').click(); }"); // start new timer
//    this->runJSinPage("$('#timer-task-picker').click();"); // task picker toggle
}

void MainWidget::stopTask()
{
    goToTimerPage();
    this->runJSinPage("if($('.btn-timer').text().trim().toLowerCase() == 'stop timer') { $('.btn-timer').click(); }");
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
    QTWEPage->runJavaScript("var task = TC.TimeTracking.getTask(angular.element(document.body).injector().get('TimerService').timer.task_id);"
                                "if(task!=null){task.name}",
    [this](const QVariant &v) {
//        qDebug() << "Timer name: " << v.toString();
        setTimerName(v.toString());
    });
}


void MainWidget::quit()
{
    QApplication::quit();
}

void MainWidget::setApiKey(const QString &apiKey) {
    MainWidget::apiKey = apiKey;
    settings.setValue(SETT_APIKEY, apiKey); // save apikey to settings
}


void MainWidget::setTimerName(const QString &timerName) {
    QFont x = QFont();
    QFontMetrics metrix(x);
    int width = 100; // pixels
    MainWidget::timerName = metrix.elidedText(timerName, Qt::ElideRight, width);
    emit timerStatusChanged(true, MainWidget::timerName); // reenable task stopping
}

void MainWidget::setIsTimerRunning(bool isTimerRunning) {
    MainWidget::timerIsRunning = isTimerRunning;
    if(isTimerRunning){
        fetchTimerName(); // this will make menu say "Stop XYZ timer"
    }else{ // no timer running
        emit timerStatusChanged(false, "timer"); // disable the option - gray it out;  make the option say "Stop timer"
    }
}

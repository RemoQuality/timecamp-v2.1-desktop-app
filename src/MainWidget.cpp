#include <QEventLoop>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include "MainWidget.h"
#include "ui_MainWidget.h"

#include "Settings.h"
#include "WindowEventsManager.h"


MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::MainWidget)
{
    ui->setupUi(this);

//    this->setAttribute(Qt::WA_TranslucentBackground);
//    this->setAutoFillBackground(true);

    QPixmap bkgnd(MAIN_BG);
//    bkgnd = bkgnd.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QPalette palette;
    palette.setBrush(QPalette::Window, bkgnd);
    this->setPalette(palette);

    // set some defaults
    loggedIn = false;

    this->setMinimumSize(QSize(350, 500));
//
//#ifdef Q_OS_MACOS
//    this->setWindowFlags(Qt::Sheet | Qt::WindowCloseButtonHint | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
//#else
//    this->setWindowFlags( Qt::WindowStaysOnTopHint );
//#endif
//
#ifdef Q_OS_MACOS
    this->setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
#else
//    this->setWindowFlags( Qt::Window );
#endif

    this->setAcceptDrops(false);

    restoreGeometry(settings.value("mainWindowGeometry").toByteArray()); // from QWidget; restore saved window position
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::init()
{
    this->setWindowTitle(WINDOW_NAME);
    this->setupWebview(); // starts the embedded webpage
    MainWidgetWasInitialised = true;
    this->wasTheWindowLeftOpened();
}

void MainWidget::handleSpacingEvents()
{
//    qInfo("Size: %d x %d", size().width(), size().height());
    if (MainWidgetWasInitialised) {
        this->setUpdatesEnabled(false);
        QTWEView->resize(size()); // resize webview
        settings.setValue("mainWindowGeometry", saveGeometry()); // save window position
        settings.sync();
        this->setUpdatesEnabled(true);
    }
}

void MainWidget::moveEvent(QMoveEvent *event)
{
    this->handleSpacingEvents();
    QWidget::moveEvent(event); // do the default "whatever happens on move"
}

void MainWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange) {
        this->handleSpacingEvents();
    }
    QWidget::changeEvent(event);
}

void MainWidget::resizeEvent(QResizeEvent *event)
{
    this->handleSpacingEvents();
    QWidget::resizeEvent(event); // do the default "whatever happens on resize"
}

void MainWidget::closeEvent(QCloseEvent *event)
{
    this->handleSpacingEvents();
    settings.setValue(SETT_WAS_WINDOW_LEFT_OPENED, false); // save if window was opened
    settings.sync();
    hide(); // hide our window when X was pressed
    emit windowStatusChanged(false);
    event->ignore(); // don't do the default action (which usually is app exit)
}

void MainWidget::twoSecTimerTimeout()
{
    if (loggedIn) {
        emit checkIsIdle();
        checkIsTimerRunning();
        fetchRecentTasks();
        if (settings.value(SETT_APIKEY).toString().isEmpty()) {
            fetchAPIkey();
        }
    } else {
        setApiKey("");
    }
}

void MainWidget::setupWebview()
{
    QTWEView = new TCWebEngineView(this);
    QTWEView->setContextMenuPolicy(Qt::NoContextMenu); // disable context menu in embedded webpage
    QTWEView->setAcceptDrops(false);
    QTWEView->setAttribute(Qt::WA_TranslucentBackground);
    QTWEView->setStyleSheet("background:transparent");
    connect(QTWEView, &QWebEngineView::loadStarted, this, &MainWidget::handleLoadStarted);
    connect(QTWEView, &QWebEngineView::loadProgress, this, &MainWidget::handleLoadProgress);
    connect(QTWEView, &QWebEngineView::loadFinished, this, &MainWidget::handleLoadFinished);


    QTWEProfile = new QWebEngineProfile(APPLICATION_NAME, QTWEView); // set "profile" as appName
    QTWEProfile->setHttpUserAgent(CONN_USER_AGENT); // add useragent to this profile

    QTWESettings = QTWEProfile->settings();
    QTWESettings->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true); // modify settings: enable Fullscreen
    //QTWESettings->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);

    TCri = new TCRequestInterceptor();
    QTWEProfile->setRequestInterceptor(TCri);

    QTWEPage = new QWebEnginePage(QTWEProfile, QTWEView);
    QTWEPage->setBackgroundColor(Qt::transparent);
    QTWEView->setPage(QTWEPage);

    refreshBind = new QShortcut(QKeySequence::Refresh, this);
    refreshBind->setContext(Qt::ApplicationShortcut);
    connect(refreshBind, &QShortcut::activated, this, &MainWidget::webviewRefresh);

    fullscreenBind = new QShortcut(QKeySequence::FullScreen, this);
    fullscreenBind->setContext(Qt::ApplicationShortcut);
    connect(fullscreenBind, &QShortcut::activated, this, &MainWidget::webviewFullscreen);

//    pagePointer = m_pWebEngineView->page();

    this->goToTimerPage(); // loads main app url
//    QTWEPage->load(QUrl(APPLICATION_URL));
//    QTWEPage->load(QUrl("http://request.urih.com/"));


    connect(QTWEPage, &QWebEnginePage::titleChanged, this, &MainWidget::webpageTitleChanged);

}

void MainWidget::handleLoadStarted()
{
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
//    qDebug() << "cursor: load started";
}

void MainWidget::handleLoadProgress(int progress)
{
//    qDebug() << "cursor: load progress " << progress;
    if (progress == 100) {
        QGuiApplication::restoreOverrideCursor(); // pop from the cursor stack
    }
}

void MainWidget::handleLoadFinished(bool ok)
{
    Q_UNUSED(ok);
    QGuiApplication::restoreOverrideCursor(); // pop from the cursor stack
//    qDebug() << "cursor: load finished " << ok;
}

void MainWidget::wasTheWindowLeftOpened()
{
    if (settings.value(SETT_WAS_WINDOW_LEFT_OPENED, true).toBool()) {
        this->open();
    }
}

void MainWidget::webpageTitleChanged(QString title)
{
//    qInfo("[NEW_TC]: Webpage title changed: ");
//    qInfo(title.toLatin1().constData());
    checkIfLoggedIn(title);
    if (!loggedIn) {
        this->runJSinPage("jQuery('#about .news').parent().parent().attr('class', 'hidden').siblings().first().attr('class', 'col-xs-12 col-sm-10 col-sm-push-1 col-md-8 col-md-push-2 col-lg-6 col-lg-push-3')");
        LastTasks.clear(); // clear last tasks
        LastTasksCache = QJsonDocument(); // clear the cache
        emit lastTasksChanged();
    }
    emit pageStatusChanged(loggedIn, title);
    this->setWindowTitle(title); // https://trello.com/c/J8dCKeV2/43-niech-tytul-apki-desktopowej-sie-zmienia-
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->checkIsTimerRunning();
}

void MainWidget::clearCache()
{
    this->setUpdatesEnabled(false);
    this->runJSinPage("localStorage.clear()");
    QTWEProfile->clearAllVisitedLinks();
    QTWEProfile->clearHttpCache();
    this->setUpdatesEnabled(true);
}

void MainWidget::webviewRefresh()
{
    qDebug("[NEW_TC]: page refresh");
    this->clearCache();
    this->goToTimerPage();
    this->forceLoadUrl(APPLICATION_URL);
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
    if (!title.toLower().contains(QRegExp("log in|login|register|create free account|create account|time tracking software|blog"))) {
        loggedIn = true;
    } else {
        loggedIn = false; // when we log out, we need to set this variable again
    }
}

void MainWidget::open()
{
    settings.setValue(SETT_WAS_WINDOW_LEFT_OPENED, true); // save if window was opened
    settings.sync();
    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
    QTWEView->resize(size()); // resize webview
    show();
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    raise();  // for MacOS
    activateWindow(); // for Windows
    raise();  // for MacOS
    emit windowStatusChanged(true);
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 256); // force waiting before we quit this func; and process some events
}

void MainWidget::runJSinPage(QString js)
{
    QTWEPage->runJavaScript(js);
}

void MainWidget::forceLoadUrl(QString url)
{
    this->runJSinPage("window.location='" + url + "'");
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
        QMetaObject::Connection conn1 = QObject::connect(QTWEPage, &QWebEnginePage::loadFinished, &loop, &QEventLoop::quit);
        QMetaObject::Connection conn2 = QObject::connect(QTWEPage, &QWebEnginePage::loadProgress, [&loop](const int &newValue)
        {
            qDebug() << "Load progress: " << newValue;
            if (newValue == 100) {
                QThread::msleep(128);
                loop.quit();
            }
        });
        QMetaObject::Connection conn3 = QObject::connect(QTWEPage, &QWebEnginePage::iconUrlChanged, [&]()
        {
            this->webpageTitleChanged(QTWEPage->title());
        });
        QTWEPage->load(QUrl(APPLICATION_URL));
        loop.exec();
        QObject::disconnect(conn1);
        QObject::disconnect(conn2);
        QObject::disconnect(conn3);
        QThread::msleep(128);

        this->webpageTitleChanged(QTWEPage->title());
    } else {
        this->refreshTimerPageData();
    }
}

void MainWidget::goToAwayPage()
{
    this->clearCache();
    this->open();

//    emit windowStatusChanged(true);
    QTWEPage->load(QUrl(OFFLINE_URL));
}

void MainWidget::startTask()
{
    this->goToTimerPage();
    this->stopTask(); // stop the last timer
    this->open();
    this->pressStartTimerButton();
//    emit windowStatusChanged(true);
}

void MainWidget::pressStartTimerButton()
{
    // if on manual time adding, switch to "start timer" button
    this->runJSinPage("if($('.btn-timer').text().trim().toLowerCase() == 'add time entry') { "
                      "$('.btn-timer').siblings('.btn-link').click();"
                      "}"
    );

    // actually start timer
    this->runJSinPage("if($('.btn-timer').text().trim().toLowerCase() == 'start timer') { "
                      "$('.btn-timer').click(); "
                      "}"); // start new timer

//    this->runJSinPage("$('#timer-task-picker').click();"); // task picker toggle is now launched atuomatically

}

void MainWidget::stopTask()
{
    this->goToTimerPage();
    this->runJSinPage("if($('.btn-timer').text().trim().toLowerCase() == 'stop timer') { $('.btn-timer').click(); }");
}

void MainWidget::startTaskByID(qint64 taskID)
{
    this->goToTimerPage();
    this->stopTask();
    QThread::msleep(128);
    this->runJSinPage("$('#timer-task-picker').click();");
    this->runJSinPage("$(\".widgetSelectTask[data-task-id='" + QString::number(taskID) + "'\")[0].click()");
    QThread::msleep(128);
    this->pressStartTimerButton();
}

void MainWidget::startTaskByTaskObj(Task* task)
{
    this->goToTimerPage();
    QTWEPage->runJavaScript("var task = TC.TimeTracking.getTask(angular.element(document.body).injector().get('TimerService').timer.task_id);"
                            "if(task!=null){task.name}", [this, task](const QVariant &v)
                            {

                                QString timerName = v.toString();
                                if(timerName != task->getName()) {
                                    this->startTaskByID(task->getTaskId());
                                }
                            });
}

void MainWidget::refreshTimerPageData()
{
    if (this->checkIfOnTimerPage()) {
        this->runJSinPage("$('.btn .fa-repeat').parent().click();");
    }
}

void MainWidget::checkIsTimerRunning()
{
    QTWEPage->runJavaScript("angular.element(document.body).injector().get('TimerService').timer.isTimerRunning", [this](
        const QVariant &v)
    {
//        qDebug() << "Timer running: " << v.toString();
        setIsTimerRunning(v.toBool());
    });
}

void MainWidget::fetchRecentTasks()
{
    QTWEPage->runJavaScript("JSON.stringify(TC.TimeTracking.Lasts)", [this](const QVariant &v)
    {
//        LastTasks.clear(); // don't need to clear a QHash

//        qDebug() << v.toString();
        QJsonDocument itemDoc = QJsonDocument::fromJson(v.toByteArray());
        if (itemDoc != LastTasksCache) {
            QJsonArray rootArray = itemDoc.array();
            for (QJsonValueRef val: rootArray) {
                QJsonObject obj = val.toObject();
//            qDebug() << obj.value("task_id").toString().toInt() << ": " << obj.value("name").toString();
                LastTasks.insert(obj.value("name").toString(), obj.value("task_id").toString().toInt());
            }
            LastTasksCache = itemDoc;
            emit lastTasksChanged();
        }
    });
}

void MainWidget::fetchAPIkey()
{
//    QTWEPage->runJavaScript("await window.apiService.getToken()",
    QTWEPage->runJavaScript("window.apiService.getToken().$$state.value", [this](const QVariant &v)
    {
//        qDebug() << "API Key: " << v.toString();
        setApiKey(v.toString());
    });
}

void MainWidget::fetchTimerName()
{
    QTWEPage->runJavaScript("var task = TC.TimeTracking.getTask(angular.element(document.body).injector().get('TimerService').timer.task_id);"
                            "if(task!=null){task.name}", [this](const QVariant &v)
                            {
//        qDebug() << "Timer name: " << v.toString();
                                setTimerName(v.toString());
                            });
}

void MainWidget::quit()
{
    QGuiApplication::quit();
}

void MainWidget::setApiKey(const QString &apiKey)
{
    settings.setValue(SETT_APIKEY, apiKey); // save apikey to settings
    settings.sync();
}

void MainWidget::setTimerName(const QString &timerName)
{
    emit timerStatusChanged(true, timerName); // reenable task stopping
}

void MainWidget::setIsTimerRunning(bool isTimerRunning)
{
    if (isTimerRunning) {
        fetchTimerName(); // this will make menu say "Stop XYZ timer"
    } else { // no timer running
        emit timerStatusChanged(false, ""); // disable the option - gray it out;
    }
}

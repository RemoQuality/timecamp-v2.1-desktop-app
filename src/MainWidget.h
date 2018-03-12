#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QMenu>
#include <QAction>
#include <QShortcut>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QUrl>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QSettings>

#include "TCRequestInterceptor.h"
#include "TCWebEngineView.h"

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(MainWidget)

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();
    void init();

    void twoSecTimerTimeout();

signals:
    void pageStatusChanged(bool, QString);
    void timerStatusChanged(bool, QString);
    void checkIsIdle();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

public slots:
    void webviewRefresh();
    void webviewFullscreen();

    void webpageTitleChanged(QString title);

    void wasTheWindowLeftOpened();
    void open();
    //void status();
    void startTask();
    void stopTask();
    void quit();

private:
    Ui::MainWidget *ui;
    QSettings settings;

    TCRequestInterceptor *TCri;

    TCWebEngineView* QTWEView;
    QWebEngineProfile *QTWEProfile;
    QWebEnginePage *QTWEPage;
    QWebEngineSettings *QTWESettings;

    void runJSinPage(QString js);
    void checkIsTimerRunning();
    void fetchAPIkey();
    void fetchTimerName();
    bool checkIfOnTimerPage();
    void goToTimerPage();


    void checkIfLoggedIn(QString title);
    void setupWebview();

    QShortcut *refreshBind;
    QShortcut *fullscreenBind;

    bool loggedIn;
    QString apiKey;
    QString timerName;
    bool timerIsRunning;

    void setApiKey(const QString &apiKey);
    void setTimerName(const QString &timerName);
    void setIsTimerRunning(bool isTimerRunning);

};

#endif // MAINWIDGET_H

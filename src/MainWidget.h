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

namespace Ui {
class MainWidget;
}

class QAction;
class QLabel;
class QMenu;

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void webviewRefresh();
    void webviewFullscreen();

    void webpageTitleChanged(QString title);

    void iconActivated(QSystemTrayIcon::ActivationReason);
    void open();
    //void status();
    void startTask();
    void stopTask();
    void autoStart(bool checked);
    void tracker(bool checked);
    void quit();

private:
    Ui::MainWidget *ui;
    QSettings settings;

    TCRequestInterceptor *TCri;

    QWebEngineView* QTWEView;
    QWebEngineProfile *QTWEProfile;
    QWebEnginePage *QTWEPage;
    QWebEngineSettings *QTWESettings;

    void runJSinPage(QString js);
    void checkIsTimerRunning();
    void fetchAPIkey();
    void fetchTimerName();

    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;

    void checkIfLoggedIn(QString title);
    void setupWebview();
    void setupTray(QWidget *parent);
    void setupSettings();

    QShortcut *refreshBind;
    QShortcut *fullscreenBind;

    void createActions(QMenu *menu);
    QAction *openAct;
    QAction *startTaskAct;
    QAction *stopTaskAct;
    QAction *trackerAct;
    QAction *autoStartAct;
    QAction *quitAct;


    bool loggedIn;
    QString apiKey;
    QString timerName;
    bool isTimerRunning;

    void setApiKey(const QString &apiKey);
    void setTimerName(const QString &timerName);
    void setIsTimerRunning(bool isTimerRunning);

};

#endif // MAINWIDGET_H

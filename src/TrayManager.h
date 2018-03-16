#ifndef THEGUI_TRAYMANAGER_H
#define THEGUI_TRAYMANAGER_H

#include <QMenu>
#include <QAction>
#include <QShortcut>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QSettings>

#include "Widget.h"

#ifdef __APPLE__
#include "Widget_M.h"
#else
#endif

class MainWidget;

class TrayManager: public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TrayManager)

public:

    static TrayManager &instance();
    explicit TrayManager(QObject *parent = nullptr);
    virtual ~TrayManager(){}

    void setupTray(MainWidget*);
    void setupSettings();

    void updateStopMenu(bool, QString);
    void loginLogout(bool, QString);
    void openCloseWindowText(bool);

signals:
    void pcActivitiesValueChanged(bool);

public slots:
    void iconActivated(QSystemTrayIcon::ActivationReason);
    void autoStart(bool checked);
    void tracker(bool checked);
    void widgetToggl(bool checked);
    void openCloseWindowAction();
    void contactSupport();


private:
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QSettings settings;

    void createActions(QMenu*);
    QAction *openAct;
    QAction *startTaskAct;
    QAction *stopTaskAct;
    QAction *trackerAct;
    QAction *autoStartAct;
    QAction *widgetAct;
    QAction *helpAct;
    QAction *quitAct;

    MainWidget *mainWidget;

    Widget *widget;
};


#endif //THEGUI_TRAYMANAGER_H

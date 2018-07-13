#ifndef THEGUI_TRAYMANAGER_H
#define THEGUI_TRAYMANAGER_H

#include <QMenu>
#include <QAction>
#include <QShortcut>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QSettings>
#include <QSignalMapper>

#include "Widget/Widget.h"

#ifdef Q_OS_MACOS

#include "Widget/Widget_M.h"
#define _WIDGET_EXISTS_

#else

#include "Widget/FloatingWidget.h"
#define _WIDGET_EXISTS_

#endif

class MainWidget;

class TrayManager : public QObject
{
Q_OBJECT
    Q_DISABLE_COPY(TrayManager)

public:

    static TrayManager &instance();
    virtual ~TrayManager() {}

    void setupTray(MainWidget *);
    void setupSettings();

    void updateStopMenu(bool, QString);
    void updateWidgetStatus(bool, QString);
    void updateWidget(bool, QString);
    void loginLogout(bool, QString);

    bool wasLoggedIn = false;
    QMenu *getTrayMenu() const;

signals:
    void pcActivitiesValueChanged(bool);
    void taskSelected(int);

public slots:
    void iconActivated(QSystemTrayIcon::ActivationReason);
    void menuActionHandler(QAction *action);
    void autoStart(bool checked);
    void tracker(bool checked);
    void updateRecentTasks();
    void openCloseWindowAction();
    void contactSupport();
#ifdef _WIDGET_EXISTS_
    void widgetToggl(bool checked);
#endif

protected:
    explicit TrayManager(QObject *parent = nullptr);

private:
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QSettings settings;

    void createActions(QMenu *);
    void assignActions(QMenu *);
    QAction *openAct;
    QAction *recentTasksTitleAct;
    QAction *startTaskAct;
    QAction *stopTaskAct;
    QAction *trackerAct;
    QAction *autoStartAct;
    QAction *widgetAct;
    QAction *helpAct;
    QAction *quitAct;

    MainWidget *mainWidget;

#ifdef _WIDGET_EXISTS_
    Widget *widget;
public:
    void setWidget(Widget *widget);
#endif

    bool areMenusEqual(QMenu *menu1, QMenu *menu2);
};


#endif //THEGUI_TRAYMANAGER_H

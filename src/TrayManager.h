#ifndef THEGUI_TRAYMANAGER_H
#define THEGUI_TRAYMANAGER_H

#include <QMenu>
#include <QAction>
#include <QShortcut>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QSettings>

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

signals:
    void pcActivitiesValueChanged(bool);

private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason);
    void autoStart(bool checked);
    void tracker(bool checked);

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
    QAction *quitAct;

    MainWidget *mainWidget;
};


#endif //THEGUI_TRAYMANAGER_H

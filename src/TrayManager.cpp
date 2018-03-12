#include <QSystemTrayIcon>
#include <QMessageBox>
#include "Settings.h"
#include "TrayManager.h"
#include "MainWidget.h"

#include "Autorun.h"

TrayManager &TrayManager::instance()
{
    static TrayManager _instance;
    return _instance;
}

TrayManager::TrayManager(QObject *parent)
    : QObject(parent)
{
}

void TrayManager::setupTray(MainWidget *parent)
{
    mainWidget = parent;
    if(!QSystemTrayIcon::isSystemTrayAvailable()){
        QMessageBox::critical(mainWidget,":(","Ninja Mode is not available on this computer. Try again later :P");
    }

    trayMenu = new QMenu();
    createActions(trayMenu);

    trayIcon = new QSystemTrayIcon(this);

    /*
    // Unbind "tray icon activates window"
     https://trello.com/c/qyCrTMfy/39-tray-kliknięcie-niech-zawsze-pokazuje-menu-otwieramy-przez-open-z-tego-menu

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
    */

    trayIcon->setIcon(QIcon(MAIN_ICON));
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();
}

void TrayManager::setupSettings()
{
    qDebug() << "Setting up settings";
    // set checkboxes
    autoStartAct->setDisabled(false);
    autoStartAct->setChecked(Autorun::checkAutorun());
    trackerAct->setChecked(settings.value(SETT_TRACK_PC_ACTIVITIES, false).toBool());

    // act on the saved settings
    this->autoStart(autoStartAct->isChecked());
    this->tracker(trackerAct->isChecked());
}

void TrayManager::updateStopMenu(bool canBeStopped, QString timerName)
{
    stopTaskAct->setText("Stop " + timerName);
    stopTaskAct->setEnabled(canBeStopped);
}

void TrayManager::autoStart(bool checked)
{
    if(checked){
        Autorun::enableAutorun();
    }else{
        Autorun::disableAutorun();
    }
}

void TrayManager::tracker(bool checked)
{
    settings.setValue(SETT_TRACK_PC_ACTIVITIES, checked);
    emit pcActivitiesValueChanged(checked);
}

void TrayManager::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason != QSystemTrayIcon::Context){
        mainWidget->open();
    }
}

void TrayManager::createActions(QMenu *menu)
{
    openAct = new QAction(tr("Open"), this);
    openAct->setStatusTip(tr("Open browser"));
    connect(openAct, &QAction::triggered, mainWidget, &MainWidget::open);

    startTaskAct = new QAction(tr("Start timer"), this);
    startTaskAct->setStatusTip(tr("Go to task selection screen"));
    startTaskAct->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_N);
    startTaskAct->setShortcutVisibleInContextMenu(true);
    startTaskAct->setShortcutContext(Qt::ApplicationShortcut);
    connect(startTaskAct, &QAction::triggered, mainWidget, &MainWidget::startTask);

    stopTaskAct = new QAction(tr("Stop timer"), this);
    stopTaskAct->setStatusTip(tr("Stop currently running timer"));
    stopTaskAct->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_M);
    stopTaskAct->setShortcutVisibleInContextMenu(true);
    stopTaskAct->setShortcutContext(Qt::ApplicationShortcut);
    connect(stopTaskAct, &QAction::triggered, mainWidget, &MainWidget::stopTask);

    trackerAct = new QAction(tr("Track computer activities"), this);
    trackerAct->setCheckable(true);
    connect(trackerAct, &QAction::triggered, this, &TrayManager::tracker);

    autoStartAct = new QAction(tr("Launch app on login"), this);
    autoStartAct->setDisabled(true); // disable by default, till we login
    autoStartAct->setCheckable(true);
    connect(autoStartAct, &QAction::triggered, this, &TrayManager::autoStart);

    quitAct = new QAction(tr("Quit"), this);
    quitAct->setStatusTip(tr("Close the app"));
    connect(quitAct, &QAction::triggered, mainWidget, &MainWidget::quit);


    menu->addAction(openAct);
    menu->addSeparator();
    menu->addAction(startTaskAct);
    menu->addAction(stopTaskAct);
    menu->addSeparator();
    menu->addAction(trackerAct);
    menu->addSeparator();
    menu->addAction(autoStartAct);
    menu->addSeparator();
    menu->addAction(quitAct);
}

void TrayManager::loginLogout(bool loggedIn, QString tooltipText)
{
    qDebug() << "Login/Logout action";
    startTaskAct->setEnabled(loggedIn);
    stopTaskAct->setEnabled(loggedIn);
    trackerAct->setEnabled(loggedIn);
    trayIcon->setToolTip(tooltipText);
    if (loggedIn) {
        this->setupSettings();
    } else {
        emit pcActivitiesValueChanged(false); // don't track PC activities
    }
}

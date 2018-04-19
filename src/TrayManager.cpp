#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QDesktopServices>

#include "Settings.h"
#include "TrayManager.h"
#include "MainWidget.h"

#include "Autorun.h"

TrayManager &TrayManager::instance()
{
    static TrayManager _instance;
    return _instance;
}

TrayManager::TrayManager(QObject *parent) : QObject(parent)
{
}

void TrayManager::setupTray(MainWidget *parent)
{
    mainWidget = parent;
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(mainWidget,
                              "No tray",

                              "We couldn't detect system tray. Please contact us at desktopapp@timecamp.com "
                              "with information about your Operating System, Desktop Environment you use (KDE, Gnome, MATE, etc) "
                              "and their respective versions. The more information you can give the better."
                              "\n\n"
                              "If you're using GNOME, try their Shell Extensions: TopIcons or AppIndicator Support.");
    }

    trayMenu = new QMenu(parent);
    createActions(trayMenu);

#ifndef Q_OS_MACOS
    trayIcon = new QSystemTrayIcon(parent);

    /*
    // Unbind "tray icon activates window"
     https://trello.com/c/qyCrTMfy/39-tray-kliknięcie-niech-zawsze-pokazuje-menu-otwieramy-przez-open-z-tego-menu

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
    */
    trayIcon->setIcon(QIcon(MAIN_ICON));
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();
#endif

#ifdef Q_OS_MACOS
    widget = new Widget_M();
    widget->setMenu(trayMenu);
    widget->setIcon(":/Icons/AppIcon_Dark.png");
    widget->setText(""); // at the start there should be no timer text
#endif
    settings.sync();
    this->setupSettings();
}

void TrayManager::setupSettings()
{
    qDebug() << "[Tray] Update checkboxes (Settings)";
    // set checkboxes
    autoStartAct->setDisabled(false);
    autoStartAct->setChecked(Autorun::checkAutorun());
    trackerAct->setChecked(settings.value(SETT_TRACK_PC_ACTIVITIES, false).toBool());
#ifdef _WIDGET_EXISTS_
    widgetAct->setChecked(settings.value(SETT_SHOW_WIDGET, true).toBool());
    this->widgetToggl(widgetAct->isChecked());
#endif
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
    if (checked) {
        Autorun::enableAutorun();
    } else {
        Autorun::disableAutorun();
    }
}

void TrayManager::tracker(bool checked)
{
    settings.setValue(SETT_TRACK_PC_ACTIVITIES, checked);
    emit pcActivitiesValueChanged(checked);
}

#ifdef _WIDGET_EXISTS_
void TrayManager::widgetToggl(bool checked)
{
    settings.setValue(SETT_SHOW_WIDGET, checked);
    if (checked) {
        widget->showMe();
    } else {
        widget->hideMe();
    }
}
#endif

void TrayManager::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason != QSystemTrayIcon::Context) {
        mainWidget->open();
    }
}

void TrayManager::openCloseWindowAction()
{
    mainWidget->open();
}

void TrayManager::contactSupport()
{
    QUrl mail("https://www.timecamp.com/kb/contact/?utm_source=timecamp_desktop");
    QDesktopServices::openUrl(mail);
};

void TrayManager::createActions(QMenu *menu)
{
    openAct = new QAction(tr("Show"), this);
    openAct->setStatusTip(tr("Opens TimeCamp interface"));
    openAct->setShortcut(QKeySequence(KB_SHORTCUTS_OPEN_WINDOW));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    openAct->setShortcutVisibleInContextMenu(true);
#endif
//    connect(openAct, &QAction::triggered, mainWidget, &MainWidget::open);
    connect(openAct, &QAction::triggered, this, &TrayManager::openCloseWindowAction);

    startTaskAct = new QAction(tr("Start timer"), this);
    startTaskAct->setStatusTip(tr("Go to task selection screen"));
    startTaskAct->setShortcut(QKeySequence(KB_SHORTCUTS_START_TIMER));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    startTaskAct->setShortcutVisibleInContextMenu(true);
#endif
    startTaskAct->setShortcutContext(Qt::ApplicationShortcut);
    connect(startTaskAct, &QAction::triggered, mainWidget, &MainWidget::startTask);

    stopTaskAct = new QAction(tr("Stop timer"), this);
    stopTaskAct->setStatusTip(tr("Stop currently running timer"));
    stopTaskAct->setShortcut(QKeySequence(KB_SHORTCUTS_STOP_TIMER));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    stopTaskAct->setShortcutVisibleInContextMenu(true);
#endif
    stopTaskAct->setShortcutContext(Qt::ApplicationShortcut);
    connect(stopTaskAct, &QAction::triggered, mainWidget, &MainWidget::stopTask);

    trackerAct = new QAction(tr("Track computer activities"), this);
    trackerAct->setCheckable(true);
    connect(trackerAct, &QAction::triggered, this, &TrayManager::tracker);

#ifdef _WIDGET_EXISTS_
    widgetAct = new QAction(tr("Time widget"), this);
    widgetAct->setCheckable(true);
    connect(widgetAct, &QAction::triggered, this, &TrayManager::widgetToggl);
#endif

    autoStartAct = new QAction(tr("Start with computer"), this);
    autoStartAct->setDisabled(true); // disable by default, till we login
    autoStartAct->setCheckable(true);
    connect(autoStartAct, &QAction::triggered, this, &TrayManager::autoStart);

    helpAct = new QAction(tr("Help && support"), this);
    helpAct->setStatusTip(tr("Need help? Talk to one of our support gurus"));
    connect(helpAct, &QAction::triggered, this, &TrayManager::contactSupport);

    quitAct = new QAction(tr("Quit"), this);
    quitAct->setStatusTip(tr("Close the app"));
    connect(quitAct, &QAction::triggered, mainWidget, &MainWidget::quit);


    menu->addAction(openAct);
    menu->addSeparator();
    menu->addAction(startTaskAct);
    menu->addAction(stopTaskAct);
    menu->addSeparator();
    menu->addAction(trackerAct);
    menu->addAction(autoStartAct);
#ifdef _WIDGET_EXISTS_
    menu->addAction(widgetAct);
#endif
    menu->addSeparator();
    menu->addAction(helpAct);
    menu->addSeparator();
    menu->addAction(quitAct);
}

void TrayManager::updateWidget(bool loggedIn, QString tooltipText)
{
#ifdef _WIDGET_EXISTS_
    widgetAct->setEnabled(loggedIn);
    if (!loggedIn) {
        widget->hideMe();
    }
    if (stopTaskAct->isEnabled()) { // if timer is running
        QString maybeTime = tooltipText.mid(0, 8); // first 8 chars: 12:23:45
        bool ok;
        maybeTime.mid(0, 2).toInt(&ok, 10); // take first two and try to make it int; if failed then it's not time
        if (ok) {
            widget->setText(maybeTime);
        } else {
            widget->setText(NO_TIMER_TEXT);
        }
        this->widgetToggl(widgetAct->isChecked());
    }
#endif
}

void TrayManager::loginLogout(bool isLoggedIn, QString tooltipText)
{
    qDebug() << "[Browser] Page changed; update whether logged in or not";

    this->updateWidget(isLoggedIn, tooltipText);
    startTaskAct->setEnabled(isLoggedIn);
    stopTaskAct->setEnabled(isLoggedIn);
    trackerAct->setEnabled(isLoggedIn);
#ifndef Q_OS_MACOS
    trayIcon->setToolTip(tooltipText); // we don't use trayIcon on macOS
#endif

    if (isLoggedIn) {
        if(!wasLoggedIn) {
            // if wasn't logged in, but is now:
            this->setupSettings(); // make context menu settings, again (after we logged in)
        }
    } else {
        emit pcActivitiesValueChanged(false); // don't track PC activities when not logged in, despite the setting
    }
    wasLoggedIn = isLoggedIn;
}

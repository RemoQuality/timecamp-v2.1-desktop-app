#include <QMessageBox>
#include <QDebug>
#include <QPixmap>
#include <QCoreApplication>
#include <QEventLoop>

#include "WindowEventsManager.h"
#include "Settings.h"


#ifdef Q_OS_LINUX
#include "DataCollector/WindowEvents_U.h"
#elif defined(Q_OS_WIN)
#include "DataCollector/WindowEvents_W.h"
#else
#include "DataCollector/WindowEvents_M.h"
#endif


WindowEventsManager &WindowEventsManager::instance()
{
    static WindowEventsManager _instance;
    return _instance;
}

WindowEventsManager::WindowEventsManager(QObject *parent) : QObject(parent)
{
#ifdef Q_OS_LINUX
    captureEventsThread = new WindowEvents_U();
#elif defined(Q_OS_WIN)
    captureEventsThread = new WindowEvents_W();
#else
    captureEventsThread = new WindowEvents_M();
#endif
    QObject::connect(captureEventsThread, &WindowEvents::noLongerAway, this, &WindowEventsManager::noLongerAway);
}

void WindowEventsManager::noLongerAway(unsigned long howLongWasAwayMS)
{
    QMessageBox msgBox;
    msgBox.setTextInteractionFlags(Qt::NoTextInteraction);
    msgBox.setIconPixmap(QPixmap(MAIN_ICON).scaledToWidth(96));
    msgBox.setText("You've been away from computer.");
    msgBox.setInformativeText("Do you want to log away time activity?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);

    // push data to server so it can show the away time in offline tab
    emit updateAfterAwayTime();

    // 128ms of events processing
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 128);

    // bring the Message to front
    msgBox.show();
    msgBox.setWindowState(Qt::WindowActive);
    msgBox.raise();  // for MacOS
    msgBox.activateWindow(); // for Windows

    // int ret = msgBox.exec(); // get result

    // use a new loop and process bg events
    QEventLoop loop;
    QMetaObject::Connection conn1 = QObject::connect(&msgBox, &QDialog::finished, &loop, &QEventLoop::quit);

    // 128ms of events processing
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 128);
    loop.exec();
    QObject::disconnect(conn1);

    int ret = msgBox.result(); // get result

    // handle returned valuefrom msgBox;
    switch (ret) {
        case QMessageBox::Yes:
            qDebug() << "[AwayPopup] QMessageBox::Yes";
            emit openAwayTimeManagement();
            break;
        case QMessageBox::No:
        default:
            qDebug() << "[AwayPopup] QMessageBox::No";
            break;
    }
}

void WindowEventsManager::startOrStopThread(bool start)
{
    if (start) {
        this->startThread();
    } else {
        this->stopThread();
    }
}

void WindowEventsManager::startThread()
{
    captureEventsThread->start();
}

void WindowEventsManager::stopThread()
{
    captureEventsThread->requestInterruption(); // if it checks for isInterruptionRequested

    int i = 0; // 8 - 1s, 16 - 2s, 24 - 3s, 32 - 4s
    while (captureEventsThread->isRunning() && i < 32) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 128); // process whatever user wants for 128ms
        captureEventsThread->wait(128); // then halt this thread for 128ms, until capture thread is stopped or 4secs pass
        i++;
    }

    if (captureEventsThread->isRunning()) {
        captureEventsThread->exit(); // if it uses QEventLoop
        if (captureEventsThread->isRunning()) {
            captureEventsThread->terminate(); // force close
        }
    }

    emit dataCollectingStopped();
}

WindowEvents *WindowEventsManager::getCaptureEventsThread() const
{
    return captureEventsThread;
}

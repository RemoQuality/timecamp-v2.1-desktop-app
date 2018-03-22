#include <QMessageBox>
#include "WindowEventsManager.h"

#ifdef __linux__
#include "WindowEvents_U.h"
#elif _WIN32
#include "WindowEvents_W.h"
#else
#include "WindowEvents_M.h"
#endif


WindowEventsManager &WindowEventsManager::instance()
{
    static WindowEventsManager _instance;
    return _instance;
}

WindowEventsManager::WindowEventsManager(QObject *parent)
    : QObject(parent)
{
#ifdef __linux__
    captureEventsThread = new WindowEvents_U();
#elif _WIN32
    captureEventsThread = new WindowEvents_W();
#else
    captureEventsThread = new WindowEvents_M();
#endif
    QObject::connect(captureEventsThread, &WindowEvents::noLongerAway, this, &WindowEventsManager::noLongerAway);
}

void WindowEventsManager::noLongerAway(unsigned long)
{
    QMessageBox msgBox;
    msgBox.setText("You've been away from computer.");
    msgBox.setInformativeText("Do you want to log away time activity?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    int ret = msgBox.exec();
    emit updateAfterAwayTime();
    switch (ret) {
        case QMessageBox::Yes:
            qDebug() << "AwayPopup QMessageBox::Yes";
            emit openAwayTimeManagement();
            break;
        case QMessageBox::No:
        default:
            qDebug() << "AwayPopup QMessageBox::No";
            break;
    }
}

void WindowEventsManager::startOrStopThread(bool start)
{
    if(start){
        this->startThread();
    }else{
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
    if(captureEventsThread->isRunning()){
        captureEventsThread->exit(); // if it uses QEventLoop
        if(captureEventsThread->isRunning()){
            captureEventsThread->terminate(); // force close
        }
    }
}
WindowEvents *WindowEventsManager::getCaptureEventsThread() const
{
    return captureEventsThread;
}

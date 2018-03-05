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

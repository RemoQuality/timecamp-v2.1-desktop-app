#ifndef WINDOWEVENTS_H
#define WINDOWEVENTS_H

#include <QThread>
#include "AppData.h"

class WindowEvents : public QThread
{
public:
    bool wasIdleLongEnough();
    void checkIdleStatus();
protected:
    virtual void run() = 0;
    virtual void logAppName(unsigned char* appName, unsigned char* windowName) = 0;
    virtual unsigned long getIdleTime() = 0;
private:
    unsigned long lastIdleTimestamp = 0;
    unsigned long currentIdleTimestamp = 0;
};

#endif // WINDOWEVENTS_H

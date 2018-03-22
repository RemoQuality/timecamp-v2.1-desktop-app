#ifndef WINDOWEVENTS_H
#define WINDOWEVENTS_H

#include <QThread>
#include <QDebug>
#include "AppData.h"

class WindowEvents : public QThread
{
Q_OBJECT
public:
    bool wasIdleLongEnough();
    void checkIdleStatus();

signals:
    void noLongerAway(unsigned long);

protected:
    virtual void run() = 0;
    virtual unsigned long getIdleTime() = 0;
    void static logAppName(QString appName, QString windowName, QString additionalInfo);
    bool isIdle = false;
private:
    unsigned long lastIdleTimestamp = 0;
    unsigned long currentIdleTimestamp = 0;
};

#endif // WINDOWEVENTS_H

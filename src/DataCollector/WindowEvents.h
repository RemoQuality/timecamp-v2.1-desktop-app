#ifndef WINDOWEVENTS_H
#define WINDOWEVENTS_H

#include <QThread>
#include <QDebug>
#include "src/AppData.h"

class WindowEvents : public QThread
{
Q_OBJECT
public:
    bool wasIdleLongEnoughToStopTracking();
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
    unsigned int switchToIdleTimeAfterMS = 2 * 60 * 1000; // 2min default
    bool shouldShowAwayPopup = true;
    unsigned int showAwayPopupAfterMS = 0; // right away
};

#endif // WINDOWEVENTS_H

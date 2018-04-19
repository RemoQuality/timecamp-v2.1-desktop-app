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
    bool isIdle = false;

signals:
    void noLongerAway(unsigned long); // Signals cannot be declared virtual

protected:
    virtual void run() = 0;
    virtual unsigned long getIdleTime() = 0;
    AppData static * logAppName(QString appName, QString windowName, QString additionalInfo);
private:
    unsigned long lastIdleTimestamp = 0;
    unsigned long currentIdleTimestamp = 0;
    unsigned int switchToIdleTimeAfterMS = 2 * 60 * 1000; // 2min default
    bool shouldShowAwayPopup = true;
    unsigned int showAwayPopupAfterMS = 0; // right away
};

#endif // WINDOWEVENTS_H

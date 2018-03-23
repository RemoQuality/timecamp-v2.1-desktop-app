#include "WindowEvents.h"
#include "Comms.h"
#include "Settings.h"

bool WindowEvents::wasIdleLongEnoughToStopTracking()
{
//    this->lastIdleTimestamp = this->currentIdleTimestamp;
    this->currentIdleTimestamp = getIdleTime();

//    unsigned long diff = currentIdleTimestamp; // - lastIdleTimestamp;
//    qDebug() << "time diff: " << currentIdleTimestamp;

    // if last was "idle enough", and current is not "idle enough"
    // then we switched from idle to active, so we can save the change now
    return currentIdleTimestamp > switchToIdleTimeAfterMS; // && currentIdleTimestamp < 10 * 1000;

}

void WindowEvents::checkIdleStatus()
{
    QSettings settings;
    switchToIdleTimeAfterMS = settings.value(QString("SETT_WEB_") + QString("idletime")).toUInt() * 60 * 1000;
    showAwayPopupAfterMS = settings.value(QString("SETT_WEB_") + QString("logofflinemin")).toUInt() * 60 * 1000;
    shouldShowAwayPopup = settings.value(QString("SETT_WEB_") + QString("logoffline")).toBool();

    lastIdleTimestamp = currentIdleTimestamp;
    bool wasPreviousIdle = lastIdleTimestamp > switchToIdleTimeAfterMS;
    bool wasIdleLongEnoughToShowAwayPopup = lastIdleTimestamp > showAwayPopupAfterMS;

    if (wasIdleLongEnoughToStopTracking()) {
        this->logAppName("IDLE", "IDLE", ""); // firstly log "IDLE" app, while not being idle
        isIdle = true; // then set the idle bit, so we don't set the app anymore
    } else if (wasPreviousIdle) {
        // was idle but is not anymore
        isIdle = false;
        if(shouldShowAwayPopup && wasIdleLongEnoughToShowAwayPopup) {
            emit noLongerAway(lastIdleTimestamp);
        }
    }
}

void WindowEvents::logAppName(QString appName, QString windowName, QString additionalInfo)
{
    qDebug("APP: %s | %s \n", appName.toLatin1().constData(), windowName.toLatin1().constData());
    AppData *app = new AppData(std::move(appName), std::move(windowName), std::move(additionalInfo));
    Comms::instance().saveApp(app);
}

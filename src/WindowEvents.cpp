#include "WindowEvents.h"
#include "Comms.h"
#include "Settings.h"

bool WindowEvents::wasIdleLongEnough()
{
//    this->lastIdleTimestamp = this->currentIdleTimestamp;
    this->currentIdleTimestamp = getIdleTime();

//    unsigned long diff = currentIdleTimestamp; // - lastIdleTimestamp;
//    qDebug() << "time diff: " << currentIdleTimestamp;

    // if last was "idle enough", and current is not "idle enough"
    // then we switched from idle to active, so we can save the change now
    return currentIdleTimestamp > 10*1000; // && currentIdleTimestamp < 10 * 1000;

}

void WindowEvents::checkIdleStatus()
{
    lastIdleTimestamp = currentIdleTimestamp;
    bool wasPreviousIdle = lastIdleTimestamp > 10 * 1000;
    if (wasIdleLongEnough()) {
        this->logAppName("IDLE", "IDLE", "");
    } else if (wasPreviousIdle) {
        // was idle but is not anymore
        emit noLongerAway(lastIdleTimestamp);
    }
}

void WindowEvents::logAppName(QString appName, QString windowName, QString additionalInfo)
{
    qDebug("APP: %s | %s \n", appName.toLatin1().constData(), windowName.toLatin1().constData());
    AppData *app = new AppData(std::move(appName), std::move(windowName), std::move(additionalInfo));
    Comms::instance().saveApp(app);
}

#include "WindowEvents.h"
#include "Comms.h"

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
    if(wasIdleLongEnough()){
        AppData *app = new AppData("IDLE", "IDLE", "");
        Comms::instance().saveApp(app);
    }
}

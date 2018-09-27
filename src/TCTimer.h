#ifndef TIMECAMPDESKTOP_TCTIMER_H
#define TIMECAMPDESKTOP_TCTIMER_H

#include <QtCore>
#include "Comms.h"

class TCTimer : public QObject
{
Q_OBJECT
    Q_DISABLE_COPY(TCTimer)

private:
    Comms *comms;
    bool isRunning;
    qint64 elapsed;
    qint64 task_id;
    qint64 entry_id;
    qint64 timer_id;
    qint64 external_task_id;
    QString name;
    QString start_time;

public:
//    TCTimer();
    TCTimer(Comms *comms);
    void start();
    void stop();
    void status();
    void decideTimerReply(QNetworkReply *reply, QByteArray buffer);
    void timerStatusReply(QByteArray buffer);
    void clearData();
};

// see Comms.h

#endif //TIMECAMPDESKTOP_TCTIMER_H

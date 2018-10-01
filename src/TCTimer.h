#ifndef TIMECAMPDESKTOP_TCTIMER_H
#define TIMECAMPDESKTOP_TCTIMER_H

#include <QtCore>
#include "Comms.h"
#include "Task.h"
#include "DbManager.h"
#include "Settings.h"

class TCTimer : public QObject
{
Q_OBJECT
    Q_DISABLE_COPY(TCTimer)

private:
    Comms *comms;
    bool isRunning = false;
    qint64 elapsed = 0;
    qint64 task_id;
    qint64 entry_id;
    qint64 timer_id;
    qint64 external_task_id;
    QString name;
    QString start_time;
    qint64 lastStatusCheck;

public:
    explicit TCTimer(Comms *comms);
    void decideTimerReply(QNetworkReply *reply, QByteArray buffer);
    void timerStatusReply(QByteArray buffer);
    void clearData();

signals:
    void timerStatusChanged(bool, QString);
    void timerElapsedSeconds(qint64);

public slots:
    void start(qint64 taskID = 0, qint64 entryID = 0, qint64 startedAtInMS = 0);
    void stop(qint64 timerID = 0, qint64 stoppedAtInMS = 0);
    void status();
    void startTaskByTaskObj(Task *task, bool force);
    void startTaskByID(qint64 taskID);
    void startTimerSlot();
    void stopTimerSlot();
};

#endif //TIMECAMPDESKTOP_TCTIMER_H

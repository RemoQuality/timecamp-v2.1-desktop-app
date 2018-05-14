#ifndef THEGUI_WINDOWEVENTSMANAGER_H
#define THEGUI_WINDOWEVENTSMANAGER_H

#include "DataCollector/WindowEvents.h"

class WindowEventsManager : public QObject
{
Q_OBJECT
    Q_DISABLE_COPY(WindowEventsManager)

public:

    static WindowEventsManager &instance();
    virtual ~WindowEventsManager() = default;
    WindowEvents *getCaptureEventsThread() const;

signals:
    void updateAfterAwayTime();
    void openAwayTimeManagement();

public slots:
    void startOrStopThread(bool startOrStop);
    void noLongerAway(unsigned long);

protected:
    explicit WindowEventsManager(QObject *parent = nullptr);

private:
    WindowEvents *captureEventsThread;
    void startThread();
    void stopThread();
};


#endif //THEGUI_WINDOWEVENTSMANAGER_H

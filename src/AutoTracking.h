#ifndef TIMECAMPDESKTOP_AUTOTRACKING_H
#define TIMECAMPDESKTOP_AUTOTRACKING_H


#include <QObject>
#include <QString>
#include <QtCore/QVector>
#include "Task.h"
#include "AppData.h"

class AutoTracking : public QObject {
Q_OBJECT
    Q_DISABLE_COPY(AutoTracking)

private:
    int taskUpdateThreshold = 30 * 1000; // in ms; prompt every X sec
    qint64 lastUpdate = 0;
public:
    qint64 getLastUpdate() const;

protected:
    explicit AutoTracking(QObject *parent = nullptr);

public:
    static AutoTracking &instance();

    Task *matchActivityToTaskKeywords(AppData *app);

public slots:
    void checkAppKeywords(AppData *app);
    void setLastUpdate(qint64 lastUpdate);

signals:
    void foundTask(Task* matchedTask, bool force);
};


#endif //TIMECAMPDESKTOP_AUTOTRACKING_H

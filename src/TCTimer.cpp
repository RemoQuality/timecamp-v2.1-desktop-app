#include "TCTimer.h"

TCTimer::TCTimer(Comms *comms)
    : comms(comms)
{
    clearData();

    lastStatusCheck = QDateTime::currentMSecsSinceEpoch();

    // we can't be calling API if we don't have the key; try to set the key
    comms->updateApiKeyFromSettings();
    // connect the generic reply from Comms
    QObject::connect(comms, &Comms::gotGenericReply, this, &TCTimer::decideTimerReply);
}

void TCTimer::start(qint64 taskID, qint64 entryID, qint64 startedAtInMS)
{
    QUrlQuery params = comms->getApiParams();
    params.addQueryItem("action", "start");
    if (taskID > 0) {
        params.addQueryItem("task_id", QString::number(taskID));
    }
    if (entryID > 0) {
        params.addQueryItem("task_id", QString::number(entryID));
    }
    if (startedAtInMS > 0) {
        params.addQueryItem("started_at", QDateTime::fromMSecsSinceEpoch(startedAtInMS).toString(Qt::ISODate).replace("T", " "));
    }
    comms->postRequest(comms->getApiUrl("/timer", "json"), params);
}

void TCTimer::stop(qint64 timerID, qint64 stoppedAtInMS)
{
    QUrlQuery params = comms->getApiParams();
    params.addQueryItem("action", "stop");
    if (timerID > 0) {
        params.addQueryItem("timer_id", QString::number(timerID));
    }
    if (stoppedAtInMS > 0) {
        params.addQueryItem("stopped_at", QDateTime::fromMSecsSinceEpoch(stoppedAtInMS).toString(Qt::ISODate).replace("T", " "));
    }
    comms->postRequest(comms->getApiUrl("/timer", "json"), params);
}

void TCTimer::status()
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (lastStatusCheck - now < 1000) { // if called less than a second ago
        return;
    }
    lastStatusCheck = now;
    QUrlQuery params = comms->getApiParams();
    params.addQueryItem("action", "status");
    comms->postRequest(comms->getApiUrl("/timer", "json"), params);
}

void TCTimer::decideTimerReply(QNetworkReply *reply, QByteArray buffer)
{
    QString stringUrl = reply->url().toString();
    if (stringUrl.contains("/timer")) {
        // this needs some better checks
        timerStatusReply(std::move(buffer));
    }
}

void TCTimer::timerStatusReply(QByteArray buffer)
{
    bool previousIsRunning = isRunning;
    QString previousName = name;
    QJsonDocument itemDoc = QJsonDocument::fromJson(buffer);

    buffer.truncate(MAX_LOG_TEXT_LENGTH);
    qDebug() << "Timer Status Response: " << buffer;

    QJsonObject rootObject = itemDoc.object();
    QJsonValue isTimerRuninngJsonValue = rootObject.value("isTimerRunning");
    isRunning = (isTimerRuninngJsonValue.isString() && (isTimerRuninngJsonValue.toString() == "true"));
    isRunning = (isTimerRuninngJsonValue.isBool() && isTimerRuninngJsonValue.toBool());
    if (isRunning) {
        QJsonValue elapsedJsonVal = rootObject.value("elapsed");
        if (elapsedJsonVal.isString()) {
            elapsed = elapsedJsonVal.toString().toInt();
        } else {
            elapsed = elapsedJsonVal.toInt();
        }
        task_id = rootObject.value("task_id").toString().toInt();
        entry_id = rootObject.value("entry_id").toString().toInt();
        timer_id = rootObject.value("timer_id").toString().toInt();
        external_task_id = rootObject.value("external_task_id").toString().toInt();
        name = rootObject.value("name").toString();
        if(name.isEmpty() && task_id != 0) {
            Task * taskObj = DbManager::instance().getTaskById(task_id);
            name = taskObj->getName();
        }
        start_time = rootObject.value("external_task_id").toString();
    } else {
        clearData();
    }
    QJsonValue newTimerId = rootObject.value("new_timer_id");
    if(!newTimerId.isUndefined() && !newTimerId.isNull()) {
        timer_id = newTimerId.toInt();
        elapsed = 1; // let's say it's been running for 1 sec, just to show some time
        isRunning = true;
    }

    if (previousIsRunning != isRunning || previousName != name) {
        emit timerStatusChanged(isRunning, name);
    }
    emit timerElapsedSeconds(elapsed);
}

void TCTimer::clearData()
{
    isRunning = false;
    elapsed = 0;
    task_id = 0;
    entry_id = 0;
    timer_id = 0;
    external_task_id = 0;
    name = QString("");
    start_time = QString("");
}

void TCTimer::startTaskByTaskObj(Task *task, bool force)
{
    if (force || timer_id == 0 || timer_id != task->getTaskId()) {
        this->start(task->getTaskId());
    }
}

void TCTimer::startTaskByID(qint64 taskID)
{
    this->start(taskID);
}

void TCTimer::startTimerSlot()
{
    this->start();
}

void TCTimer::stopTimerSlot()
{
    this->stop();
}

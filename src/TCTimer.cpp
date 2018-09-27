#include "TCTimer.h"
#include "Settings.h"

TCTimer::TCTimer(Comms *comms)
    : comms(comms)
{
    clearData();
    QMetaObject::Connection conn = QObject::connect(comms, &Comms::gotGenericReply, this, &TCTimer::decideTimerReply);
}

void TCTimer::start()
{

}

void TCTimer::stop()
{

}

void TCTimer::status()
{
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
    QJsonDocument itemDoc = QJsonDocument::fromJson(buffer);

    buffer.truncate(MAX_LOG_TEXT_LENGTH);
    qDebug() << "Timer Status Response: " << buffer;

    QJsonObject rootObject = itemDoc.object();
    isRunning = (rootObject.value("isTimerRunning").toString() == "true"); // can't cast or convert to Bool, so we compare the strings
    if (isRunning) {
        elapsed = rootObject.value("elapsed").toString().toInt();
        task_id = rootObject.value("task_id").toString().toInt();
        entry_id = rootObject.value("entry_id").toString().toInt();
        timer_id = rootObject.value("timer_id").toString().toInt();
        external_task_id = rootObject.value("external_task_id").toString().toInt();
        name = rootObject.value("name").toString();
        start_time = rootObject.value("external_task_id").toString();
    } else {
        clearData();
    }
}

void TCTimer::clearData()
{
    elapsed = 0;
    task_id = 0;
    entry_id = 0;
    timer_id = 0;
    external_task_id = 0;
    name = QString("");
    start_time = QString("");
}

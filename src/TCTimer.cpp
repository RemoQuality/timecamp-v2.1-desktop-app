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
    if (!comms->updateApiKeyFromSettings()) {
        return;
    }

    QUrlQuery params;
    params.addQueryItem("api_token", comms->getApiKey());
    params.addQueryItem("service", SETT_API_SERVICE_FIELD);

    params.addQueryItem("action", "status");

    QUrl serviceURL(comms->getApiUrl("/timer", "json"));
    QNetworkRequest request(serviceURL);

    QUrl URLParams;
    URLParams.setQuery(params);
    QByteArray jsonString = URLParams.toEncoded();
    QByteArray postDataSize = QByteArray::number(jsonString.size());

    // make it "www form" because thats what API expects; and add length
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    request.setRawHeader("Content-Length", postDataSize);

    comms->netRequest(request, QNetworkAccessManager::PostOperation, jsonString);
}

void TCTimer::decideTimerReply(QNetworkReply *reply)
{
    QString stringUrl = reply->url().toString();
    if(stringUrl.contains("/timer"))
    {
        // this needs some better checks
        timerStatusReply(reply);
    }
}

void TCTimer::timerStatusReply(QNetworkReply *reply)
{
    QByteArray buffer = reply->readAll();
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

#include "Comms.h"
#define SETT_APIKEY "API_KEY"


Comms &Comms::instance()
{
    static Comms _instance;
    return _instance;
}

Comms::Comms(QObject *parent) : QObject(parent) {

}

void Comms::saveApp(AppData *app) {
//    qDebug() << "getAppName: " << app->getAppName();
//    qDebug() << "getWindowName: " << app->getWindowName();
    if(Comms::instance().lastApp == NULL || Comms::instance().lastAppTimestamp == NULL){
        qDebug() << "FIRST SAVE APP RUN";
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        Comms::instance().lastAppTimestamp = now;
        Comms::instance().lastApp = app;
        return;
    }

    bool needsReporting = false;
    if (app->getAppName() != &Comms::instance().lastApp->getAppName()) {
        needsReporting = true;
    }
    if (app->getWindowName() != &Comms::instance().lastApp->getWindowName()) {
        needsReporting = true;
    }
//    qDebug() << "Needs reporting: " << needsReporting;

    if (needsReporting) {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        notifyOfApp(Comms::instance().lastApp, Comms::instance().lastAppTimestamp, now);
        Comms::instance().lastApp = app; // update app
        Comms::instance().lastAppTimestamp = now;
    }
}

void Comms::notifyOfApp(AppData *app, qint64 start, qint64 end){
    qDebug() << "[NOTIFY OF APP]";
    qDebug() << "getAppName: " << app->getAppName();
    qDebug() << "getWindowName: " << app->getWindowName();

    // read api key from settings
    QString apiKey = settings.value(SETT_APIKEY).toString();

    if(apiKey == ""){
        qDebug() << "[EMPTY API KEY !!!]";
        return;
    }

    QUrlQuery params;
    params.addQueryItem("api_token", apiKey);

    params.addQueryItem("computer_activities[0][application_name]",app->getAppName());
    params.addQueryItem("computer_activities[0][window_title]",app->getWindowName());

    QString start_time = QDateTime::fromMSecsSinceEpoch(start).toString(Qt::ISODate).replace("T", " ");
    qDebug() << "start_time: " << start_time;
    params.addQueryItem("computer_activities[0][start_time]", start_time);

    QString end_time = QDateTime::fromMSecsSinceEpoch(end).toString(Qt::ISODate).replace("T", " ");
    qDebug() << "end_time: " << end_time;
    params.addQueryItem("computer_activities[0][end_time]", end_time);

    QUrl serviceURL("https://www.timecamp.com/third_party/api/activity/api_token/" + apiKey);
    QNetworkRequest request(serviceURL);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    QUrl URLParams;
    URLParams.setQuery(params);
    QByteArray jsonString = URLParams.toEncoded();
    QByteArray postDataSize = QByteArray::number(jsonString.size());


    //request.setRawHeader("User-Agent", "My app name v0.1");
    //request.setRawHeader("X-Custom-User-Agent", "My app name v0.1");
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    request.setRawHeader("Content-Length", postDataSize);

    QNetworkAccessManager *m_qnam = new QNetworkAccessManager();
    m_qnam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    connect(m_qnam, SIGNAL(finished(QNetworkReply*)), this, SLOT(serviceRequestFinished(QNetworkReply*)));


//    qDebug() << "JSON String: " << jsonString;

// Use QNetworkReply * QNetworkAccessManager::post(const QNetworkRequest & request, const QByteArray & data); to send your request. Qt will rearrange everything correctly.
    QNetworkReply * reply = m_qnam->post(request, jsonString);

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
}

void Comms::serviceRequestFinished(QNetworkReply* reply){
    QByteArray buffer = reply->readAll();
    qDebug() << "Response: " << buffer;
}
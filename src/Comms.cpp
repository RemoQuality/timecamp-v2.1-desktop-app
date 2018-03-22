#include "Comms.h"
#include "Settings.h"

#include "DbManager.h"

#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkRequest>

#include <QUrlQuery>
#include <QEventLoop>
#include <QTimer>

Comms &Comms::instance()
{
    static Comms _instance;
    return _instance;
}

Comms::Comms(QObject *parent)
    : QObject(parent)
{
    apiKey = settings.value(SETT_APIKEY).toString();
}

void Comms::timedUpdates()
{
    lastSync = settings.value(SETT_LAST_SYNC, 0).toLongLong(); // set our variable to value from settings (so it works between app restarts)

    qDebug() << "last sync: " << lastSync;

    setCurrentTime(QDateTime::currentMSecsSinceEpoch()); // time of DB fetch is passed, so we can update to it if successful

    QList<AppData*> appList = DbManager::instance().getAppsSinceLastSync(lastSync); // get apps since last sync

    qDebug() << "app list length: " << appList.length();
    if(appList.length() > 0){ // send only if there is anything
        sendAppData(&appList);
    }
}

void Comms::saveApp(AppData *app)
{
    if (lastApp == NULL) {
        qDebug() << "[FIRST APP DETECTED]";
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        lastApp = app;
        app->setStart(now);
        return;
    }

    if (app->getAdditionalInfo() != "") {
        app->setAppName("Internet");
    }

    bool needsReporting = false;
    if (0 != QString::compare(app->getAppName(), lastApp->getAppName())) {
        needsReporting = true;
    }
    if (0 != QString::compare(app->getWindowName(), lastApp->getWindowName())) {
        needsReporting = true;
    }

//    qDebug() << "Needs reporting: " << needsReporting;

    if (needsReporting) {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
//        sendAppData(lastApp, lastAppTimestamp, now);
        lastApp->setEnd(now); // it already has start, now we only update end
        DbManager::instance().saveAppToDb(lastApp);

        app->setStart(now);
        lastApp = app; // update app reference
//        qDebug("SAVE: %s | %s \n", lastApp->getAppName().toLatin1().constData(), lastApp->getWindowName().toLatin1().constData());
    }
}

void Comms::sendAppData(QList<AppData*> *appList)
{

    // read api key from settings
    apiKey = settings.value(SETT_APIKEY).toString();

    if (apiKey.isEmpty()) {
        qInfo() << "[EMPTY API KEY !!!]";
        return;
    }

    QUrlQuery params;
    params.addQueryItem("api_token", apiKey);

    int count = 0;
    QString computer_activities = "computer_activities";

    for (AppData *app: *appList) {
//    qDebug() << "[NOTIFY OF APP]";
//    qDebug() << "getAppName: " << app->getAppName();
//    qDebug() << "getWindowName: " << app->getWindowName();
//    qDebug() << "getAdditionalInfo: " << app->getAdditionalInfo();
//    qDebug() << "getDomainFromAdditionalInfo: " << app->getDomainFromAdditionalInfo();
//    qDebug() << "getStart: " << app->getStart();
//    qDebug() << "getEnd: " << app->getEnd();

        if(app->getAppName() != "IDLE" && app->getWindowName() != "IDLE") {
            params.addQueryItem(computer_activities + "[" + QString::number(count) + "][application_name]", app->getAppName());
            params.addQueryItem(computer_activities + "[" + QString::number(count) + "][window_title]", app->getWindowName());
            if (app->getAdditionalInfo() != "") {
                params.addQueryItem(computer_activities + "[" + QString::number(count) + "][website_domain]", app->getDomainFromAdditionalInfo());
            }
            // "Web Browser App" when appName is Internet but no domain

            QString start_time = QDateTime::fromMSecsSinceEpoch(app->getStart()).toString(Qt::ISODate).replace("T", " ");
//            qDebug() << "converted start_time: " << start_time;
            params.addQueryItem(computer_activities + "[" + QString::number(count) + "][start_time]", start_time);

            QString end_time = QDateTime::fromMSecsSinceEpoch(app->getEnd()).toString(Qt::ISODate).replace("T", " ");
//            qDebug() << "converted end_time: " << end_time;
            params.addQueryItem(QString(computer_activities + "[" + QString::number(count) + "][end_time]"), end_time);
            count++;
        }
    }

//    qDebug() << "--------------";
//    qDebug() << "URL params: ";
//    qDebug() << params.toString();
//    qDebug() << "--------------\n";

    QUrl serviceURL("https://www.timecamp.com/third_party/api/activity/api_token/" + apiKey);
    QNetworkRequest request(serviceURL);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    QUrl URLParams;
    URLParams.setQuery(params);
    QByteArray jsonString = URLParams.toEncoded();
    QByteArray postDataSize = QByteArray::number(jsonString.size());

    // set up connection parameters
    // identify as our app
    request.setRawHeader("User-Agent", CONN_USER_AGENT);
    request.setRawHeader(CONN_CUSTOM_HEADER_NAME, CONN_CUSTOM_HEADER_VALUE);
    // make it "www form" because thats what API expects; and add length
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    request.setRawHeader("Content-Length", postDataSize);

    QNetworkAccessManager *m_qnam = new QNetworkAccessManager();
    m_qnam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
//    connect(m_qnam, SIGNAL(finished(QNetworkReply *)), this, SLOT(serviceRequestFinished(QNetworkReply *)));
    connect(m_qnam, &QNetworkAccessManager::finished, this, &Comms::serviceRequestFinished);


//    qDebug() << "JSON String: " << jsonString;

// Use QNetworkReply * QNetworkAccessManager::post(const QNetworkRequest & request, const QByteArray & data); to send your request. Qt will rearrange everything correctly.
    QNetworkReply *reply = m_qnam->post(request, jsonString);

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
}

void Comms::serviceRequestFinished(QNetworkReply *reply)
{
    QByteArray buffer = reply->readAll();
    qDebug() << "Response: " << buffer;
    if(buffer == ""){
        qDebug() << "update last sync to whenever we sent the data";
        settings.setValue(SETT_LAST_SYNC, getCurrentTime()); // update last sync to whenever we sent the data
    }
}

qint64 Comms::getCurrentTime() const
{
    return currentTime;
}

void Comms::setCurrentTime(qint64 current_time)
{
    Comms::currentTime = current_time;
}

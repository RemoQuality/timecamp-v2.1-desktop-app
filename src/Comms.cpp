#include "Comms.h"
#include "Settings.h"

#include "DbManager.h"

#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkRequest>

#include <QUrlQuery>
#include <QEventLoop>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

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
    getUserInfo();
    getSettings();
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

bool Comms::isApiKeyOK()
{
    if (apiKey.isEmpty()) {
        qInfo() << "[EMPTY API KEY !!!]";
        return false;
    }
    return true;
}

void Comms::sendAppData(QList<AppData*> *appList)
{
    // read api key from settings
    apiKey = settings.value(SETT_APIKEY).toString();

    if (!isApiKeyOK()) {
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
//    connect(m_qnam, SIGNAL(finished(QNetworkReply *)), this, SLOT(appDataReply(QNetworkReply *)));
    connect(m_qnam, &QNetworkAccessManager::finished, this, &Comms::appDataReply);


//    qDebug() << "JSON String: " << jsonString;

// Use QNetworkReply * QNetworkAccessManager::post(const QNetworkRequest & request, const QByteArray & data); to send your request. Qt will rearrange everything correctly.
    QNetworkReply *reply = m_qnam->post(request, jsonString);

//    QEventLoop loop;
//    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
//    loop.exec();
}

void Comms::appDataReply(QNetworkReply *reply)
{
    QByteArray buffer = reply->readAll();
    qDebug() << "AppData Response: " << buffer;
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

void Comms::getUserInfo()
{
    // read api key from settings
    apiKey = settings.value(SETT_APIKEY).toString();

    if (!isApiKeyOK()) {
        return;
    }

    QUrl serviceURL("https://www.timecamp.com/third_party/api/user/api_token/" + apiKey + "/format/json");
    QNetworkRequest request(serviceURL);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    // set up connection parameters
    // identify as our app
    request.setRawHeader("User-Agent", CONN_USER_AGENT);
    request.setRawHeader(CONN_CUSTOM_HEADER_NAME, CONN_CUSTOM_HEADER_VALUE);

    QNetworkAccessManager *m_qnam = new QNetworkAccessManager();
    m_qnam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    connect(m_qnam, &QNetworkAccessManager::finished, this, &Comms::userInfoReply);

    QNetworkReply *reply = m_qnam->get(request);

//    QEventLoop loop;
//    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
//    loop.exec();
}

void Comms::userInfoReply(QNetworkReply *reply)
{
    QByteArray buffer = reply->readAll();
    qDebug() << "UserInfo Response: " << buffer;

    QJsonDocument itemDoc = QJsonDocument::fromJson(buffer);
    QJsonObject rootObject = itemDoc.object();

    user_id = rootObject.value("user_id").toString().toInt();
    root_group_id = rootObject.value("root_group_id").toString().toInt();
    primary_group_id = rootObject.value("primary_group_id").toString().toInt();

    settings.setValue("SETT_USER_ID", user_id);
    settings.setValue("SETT_ROOT_GROUP_ID", root_group_id);
    settings.setValue("SETT_PRIMARY_GROUP_ID", primary_group_id);
    settings.sync();
    qDebug() << "SETT user_id: " << settings.value("SETT_USER_ID").toInt();
    qDebug() << "SETT root_group_id: " << settings.value("SETT_ROOT_GROUP_ID").toInt();
    qDebug() << "SETT primary_group_id: " << settings.value("SETT_PRIMARY_GROUP_ID").toInt();
}

void Comms::getSettings()
{
    // read api key from settings
    apiKey = settings.value(SETT_APIKEY).toString();

    if (!isApiKeyOK()) {
        return;
    }

//    primary_group_id = 134214;
    QString primary_group_id_str = settings.value("SETT_PRIMARY_GROUP_ID").toString();

    QUrl serviceURL(QString("https://www.timecamp.com/third_party/api/") + "group/" + primary_group_id_str + "/setting" + "/api_token/" + apiKey + "/format/json/");

    QUrlQuery params;
    params.addQueryItem("api_token", apiKey);

    // dapp settings
    params.addQueryItem("name[]", "close_agent"); // bool: can close app?

    params.addQueryItem("name[]", "pause_tracking"); // int: can have "Private Time"? if so, then X limit

    params.addQueryItem("name[]", "idletime"); // int: how quick app goes into idle

    params.addQueryItem("name[]", "logoffline"); // bool: show "away popup"?
    params.addQueryItem("name[]", "logofflinemin"); // int: after how much of idle we start showing "away popup"

    params.addQueryItem("name[]", "offlineallow"); // // bool: is the offlinecustom Array set
    params.addQueryItem("name[]", "offlinecustom"); // Array(String): names of activities for "away popup"

    params.addQueryItem("name[]", "collectWindowTitles"); // bool: save windowTitles?
    params.addQueryItem("name[]", "logOnlyActivitiesWithTasks"); // bool: tracking only when task selected
    params.addQueryItem("name[]", "dontCollectComputerActivity"); // bool: send only "active" or "nonactive"
    params.addQueryItem("name[]", "form_scheduler"); // bool: "Allow users to log overtime activities"
    params.addQueryItem("name[]", "make_screenshots"); // bool: take screenshots?

    params.addQueryItem("name[]", "group_working_time_limit"); // Array(int): stop tracking after "daily hours limit"

    // auto mode:
    params.addQueryItem("name[]", "tt_window_on_no_task"); // bool: "Display a window to choose a task, when Agent can't match any keyword in Auto Mode"
    params.addQueryItem("name[]", "turnoff_tt_after"); // int: auto tracking off, after X

    // probably server only:
//    params.addQueryItem("name[]", "unset_concurrent_apps"); // bool: "Dismiss computer activities overlapping other computer activities that are already logged"
//    params.addQueryItem("name[]", "limited_offline"); // bool: "Do not allow adding away time activity before first and after last activity on a computer"
//    params.addQueryItem("name[]", "disableDataSplit"); // bool: "can users split their away time breaks"


    serviceURL.setQuery(params.query());

//    qDebug() << "Query URL: " << serviceURL;

    QNetworkRequest request(serviceURL);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    // set up connection parameters
    // identify as our app
    request.setRawHeader("User-Agent", CONN_USER_AGENT);
    request.setRawHeader(CONN_CUSTOM_HEADER_NAME, CONN_CUSTOM_HEADER_VALUE);

    QNetworkAccessManager *m_qnam = new QNetworkAccessManager();
    m_qnam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    connect(m_qnam, &QNetworkAccessManager::finished, this, &Comms::settingsReply);

    QNetworkReply *reply = m_qnam->get(request);

//    QEventLoop loop;
//    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
//    loop.exec();
}

void Comms::settingsReply(QNetworkReply *reply) {
    QByteArray buffer = reply->readAll();
    qDebug() << "Settings Response: " << buffer;

    QJsonDocument itemDoc = QJsonDocument::fromJson(buffer);
    QJsonArray rootArray = itemDoc.array();
    for (QJsonValueRef val: rootArray) {
        QJsonObject obj = val.toObject();
//        qDebug() << obj.value("name").toString() << ": " << obj.value("value").toString();
        settings.setValue(QString("SETT_WEB_") + obj.value("name").toString(), obj.value("value").toString()); // save web settings to our settingsstore
    }
    settings.sync();

    qDebug() << "SETT idletime: " << settings.value(QString("SETT_WEB_") + QString("idletime")).toInt();
    qDebug() << "SETT logoffline: " << settings.value(QString("SETT_WEB_") + QString("logoffline")).toBool();
    qDebug() << "SETT logofflinemin: " << settings.value(QString("SETT_WEB_") + QString("logofflinemin")).toInt();
}

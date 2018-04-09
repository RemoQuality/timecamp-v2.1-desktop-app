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

Comms::Comms(QObject *parent) : QObject(parent)
{
    apiKey = settings.value(SETT_APIKEY).toString();
}

void Comms::timedUpdates()
{
    lastSync = settings.value(SETT_LAST_SYNC, 0).toLongLong(); // set our variable to value from settings (so it works between app restarts)

    qDebug() << "last sync: " << lastSync;

    setCurrentTime(QDateTime::currentMSecsSinceEpoch()); // time of DB fetch is passed, so we can update to it if successful

    QList<AppData *> appList = DbManager::instance().getAppsSinceLastSync(lastSync); // get apps since last sync

    qDebug() << "app list length: " << appList.length();
    if (appList.length() > 0) { // send only if there is anything
        sendAppData(&appList);
    }
    getUserInfo();
    getSettings();
}

void Comms::saveApp(AppData *app)
{
    if (lastApp == nullptr) {
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

    if (needsReporting) {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        lastApp->setEnd(now); // it already has start, now we only update end
        DbManager::instance().saveAppToDb(lastApp);

        app->setStart(now);
        lastApp = app; // update app reference
//        qDebug("DBSAVE: %s | %s \n", lastApp->getAppName().toLatin1().constData(), lastApp->getWindowName().toLatin1().constData());
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

void Comms::sendAppData(QList<AppData *> *appList)
{
    // read api key from settings
    apiKey = settings.value(SETT_APIKEY).toString();

    if (!isApiKeyOK()) {
        return;
    }

    bool canSendActivityInfo = !settings.value(QString("SETT_WEB_") + QString("dontCollectComputerActivity")).toBool();
    bool canSendWindowTitles = settings.value(QString("SETT_WEB_") + QString("collectWindowTitles")).toBool();

    QUrlQuery params;
    params.addQueryItem("api_token", apiKey);

    int count = 0;

    for (AppData *app: *appList) {
//    qDebug() << "[NOTIFY OF APP]";
//    qDebug() << "getAppName: " << app->getAppName();
//    qDebug() << "getWindowName: " << app->getWindowName();
//    qDebug() << "getAdditionalInfo: " << app->getAdditionalInfo();
//    qDebug() << "getDomainFromAdditionalInfo: " << app->getDomainFromAdditionalInfo();
//    qDebug() << "getStart: " << app->getStart();
//    qDebug() << "getEnd: " << app->getEnd();

        if (app->getAppName() != "IDLE" && app->getWindowName() != "IDLE") {
            QString base_str = QString("computer_activities") + QString("[") + QString::number(count) + QString("]");

            if (canSendActivityInfo) {
                params.addQueryItem(base_str + QString("[application_name]"), app->getAppName());
                if (canSendWindowTitles) {
                    params.addQueryItem(base_str + QString("[window_title]"), app->getWindowName());

                    // "Web Browser App" when appName is Internet but no domain
                    if (app->getAdditionalInfo() != "") {
                        params.addQueryItem(base_str + QString("[website_domain]"), app->getDomainFromAdditionalInfo());
                    }
                } else {
                    params.addQueryItem(base_str + QString("[window_title]"), "");
                }
            } else { // can't send activity info, collect_computer_activities == 0
                params.addQueryItem(base_str + QString("[application_name]"), "computer activity");
                params.addQueryItem(base_str + QString("[window_title]"), "");
            }

            QString start_time = QDateTime::fromMSecsSinceEpoch(app->getStart()).toString(Qt::ISODate).replace("T", " ");
            params.addQueryItem(base_str + QString("[start_time]"), start_time);
//            qDebug() << "converted start_time: " << start_time;

            QString end_time = QDateTime::fromMSecsSinceEpoch(app->getEnd()).toString(Qt::ISODate).replace("T", " ");
            params.addQueryItem(base_str + QString("[end_time]"), end_time);
//            qDebug() << "converted end_time: " << end_time;
            count++;
        }
        lastSync = app->getEnd(); // set our internal variable to value from last app
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

    this->netRequest(request, QNetworkAccessManager::PostOperation, &Comms::appDataReply, jsonString);
}

void Comms::appDataReply(QNetworkReply *reply)
{
    if(reply->error() != QNetworkReply::NoError){
        qInfo() << "Network error: " << reply->errorString();
        return;
    }

    QByteArray buffer = reply->readAll();
    qDebug() << "AppData Response: " << buffer;
    if (buffer == "") {
        qDebug() << "update last sync to whenever we sent the data";
        settings.setValue(SETT_LAST_SYNC, lastSync); // update last sync to our internal variable (to the last app in the last set)
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

    this->netRequest(request, QNetworkAccessManager::GetOperation, &Comms::userInfoReply, "");
}

void Comms::userInfoReply(QNetworkReply *reply)
{
    if(reply->error() != QNetworkReply::NoError){
        qInfo() << "Network error: " << reply->errorString();
        return;
    }

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
    qInfo() << "SETT user_id: " << settings.value("SETT_USER_ID").toInt();
    qInfo() << "SETT root_group_id: " << settings.value("SETT_ROOT_GROUP_ID").toInt();
    qInfo() << "SETT primary_group_id: " << settings.value("SETT_PRIMARY_GROUP_ID").toInt();
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

    params.addQueryItem("name[]", "dontCollectComputerActivity"); // bool: BOOL_COLLECT_COMPUTER_ACTIVITIES if true, send only "computer activity"
    params.addQueryItem("name[]", "collectWindowTitles"); // bool: save windowTitles?
    params.addQueryItem("name[]", "logOnlyActivitiesWithTasks"); // bool: tracking only when task selected
    params.addQueryItem("name[]", "make_screenshots"); // bool: take screenshots?

    params.addQueryItem("name[]", "group_working_time_limit"); // Array(int): stop tracking after "daily hours limit"

    // auto mode:
    params.addQueryItem("name[]", "tt_window_on_no_task"); // bool: "Display a window to choose a task, when Agent can't match any keyword in Auto Mode"
    params.addQueryItem("name[]", "turnoff_tt_after"); // int: auto tracking off, after X

    // probably server only:
//    params.addQueryItem("name[]", "form_scheduler"); // bool: "Allow users to log overtime activities"
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

    this->netRequest(request, QNetworkAccessManager::GetOperation, &Comms::settingsReply, "");
}

void Comms::settingsReply(QNetworkReply *reply)
{
    if(reply->error() != QNetworkReply::NoError){
        qInfo() << "Network error: " << reply->errorString();
        return;
    }
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

    qInfo() << "SETT idletime: " << settings.value(QString("SETT_WEB_") + QString("idletime")).toInt();
    qInfo() << "SETT logoffline: " << settings.value(QString("SETT_WEB_") + QString("logoffline")).toBool();
    qInfo() << "SETT logofflinemin: " << settings.value(QString("SETT_WEB_") + QString("logofflinemin")).toInt();
    qInfo() << "SETT dontCollectComputerActivity: "
            << settings.value(QString("SETT_WEB_") + QString("dontCollectComputerActivity")).toBool();
    qInfo() << "SETT collectWindowTitles: "
            << settings.value(QString("SETT_WEB_") + QString("collectWindowTitles")).toBool();
}

void Comms::netRequest(QNetworkRequest request, QNetworkAccessManager::Operation netOp, ReplyHandler callback, QByteArray data)
{
    auto *qnam = new QNetworkAccessManager();
    qnam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    connect(qnam, &QNetworkAccessManager::finished, this, callback);

    qDebug() << "Network op: " << netOp;
    qDebug() << "Request URL: " << request.url().toString();

    QNetworkReply *reply = nullptr;
    if(netOp == QNetworkAccessManager::GetOperation) {
        reply = qnam->get(request);
    }else if(netOp == QNetworkAccessManager::PostOperation) {
        reply = qnam->post(request, data);
    }

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    delete reply;
    delete qnam;
//    reply->deleteLater(); // this was sometimes deleting wrong reply!
//    qnam->deleteLater(); // and this was dropping wrong qnam...
}
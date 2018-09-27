#ifndef COMMS_H
#define COMMS_H

#include <QObject>
#include <QSettings>
#include <QNetworkReply>

#include "AppData.h"
#include "Task.h"

class Comms : public QObject
{
Q_OBJECT
    Q_DISABLE_COPY(Comms)

public:

    static Comms &instance();
    ~Comms() override = default;

    void saveApp(AppData *app);
    void sendAppData(QVector<AppData> *appList);
    void getUserInfo();
    void getSettings();
    void getTasks();

    qint64 getCurrentTime() const;
    void setCurrentTime(qint64 current_time);
    void timedUpdates();
    void tryToSendAppData();

    void netRequest(QNetworkRequest, QNetworkAccessManager::Operation = QNetworkAccessManager::GetOperation, QByteArray = nullptr);
    void postRequest(QUrl endpointUrl, QUrlQuery params);

    bool updateApiKeyFromSettings();

    QUrlQuery getApiParams();
    QUrl getApiUrl(QString, QString);
    const QString &getApiKey() const;

protected:
    explicit Comms(QObject *parent = nullptr);

private:
    AppData *lastApp = nullptr;
    QSettings settings;
    qint64 lastSync;
    qint64 currentTime;
    QString apiKey;
    int retryCount = 0;
    bool lastBatchBig = false;

    int user_id;
    int root_group_id;
    int primary_group_id;
    QNetworkAccessManager qnam;
    QHash<QUrl, std::function<void(Comms *, QByteArray buffer)>> commsReplies; // see https://stackoverflow.com/a/7582574/8538394

signals:
    void DbSaveApp(AppData *);
    void gotGenericReply(QNetworkReply *reply, QByteArray buffer);

public slots:
    void appDataReply(QByteArray buffer);
    void userInfoReply(QByteArray buffer);
    void settingsReply(QByteArray buffer);
    void tasksReply(QByteArray buffer);
    void genericReply(QNetworkReply *reply);
    void checkBatchSize();
    void clearLastApp();
};

#endif // COMMS_H

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

    void netRequest(QNetworkRequest, QNetworkAccessManager::Operation, QByteArray);

    bool updateApiKeyFromSettings();

    QUrl getApiUrl(QString, QString);
    const QString &getApiKey() const;

protected:
    explicit Comms(QObject *parent = nullptr);

private:
    AppData *lastApp;
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
    QHash<QUrl, std::function<void(Comms *, QNetworkReply *)>> commsReplies; // see https://stackoverflow.com/a/7582574/8538394

signals:
    void DbSaveApp(AppData *);
    void gotGenericReply(QNetworkReply *);

public slots:
    void appDataReply(QNetworkReply *reply);
    void userInfoReply(QNetworkReply *reply);
    void settingsReply(QNetworkReply *reply);
    void tasksReply(QNetworkReply *reply);
    void genericReply(QNetworkReply *reply);
    void checkBatchSize();
    void clearLastApp();
};

#endif // COMMS_H

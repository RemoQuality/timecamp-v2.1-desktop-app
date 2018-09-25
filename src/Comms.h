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

    void netRequest(QNetworkRequest, QNetworkAccessManager::Operation,
                    void (Comms::*)(QNetworkReply *), QByteArray = nullptr);

    bool updateApiKeyFromSettings();

    QString getApiUrl(QString, QString);
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

typedef void (Comms::*ReplyHandler)(QNetworkReply *reply);
// https://isocpp.org/wiki/faq/pointers-to-members#typedef-for-ptr-to-memfn

#endif // COMMS_H

#ifndef COMMS_H
#define COMMS_H

#include <QObject>
#include <QDateTime>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QUrlQuery>
#include <QVariant>
#include <QEventLoop>

#include "AppData.h"

class Comms : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(Comms)

public:

    static Comms &instance();
    explicit Comms(QObject *parent = nullptr);
    virtual ~Comms(){}

    void saveApp(AppData *app);
    void notifyOfApp(AppData *app, qint64 start, qint64 end);

private:
    AppData *lastApp;
    qint64 lastAppTimestamp;

signals:

public slots:
    void serviceRequestFinished(QNetworkReply* reply);
};

#endif // COMMS_H

#ifndef COMMS_H
#define COMMS_H

#include <QObject>
#include <QSettings>
#include <QNetworkReply>

#include "AppData.h"

class Comms : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(Comms)

public:

    static Comms &instance();
    explicit Comms(QObject *parent = nullptr);
    virtual ~Comms(){}

    void saveApp(AppData *app);
    void Comms::sendAppData(QList<AppData*> *appList);

private:
    AppData *lastApp;
    QSettings settings;

signals:

public slots:
    void serviceRequestFinished(QNetworkReply* reply);
};

#endif // COMMS_H

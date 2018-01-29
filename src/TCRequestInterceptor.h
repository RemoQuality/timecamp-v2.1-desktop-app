#ifndef TCREQUESTINTERCEPTOR_H
#define TCREQUESTINTERCEPTOR_H

#include <QObject>
#include <QWebEngineUrlRequestInterceptor>

class TCRequestInterceptor : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT

public:
    TCRequestInterceptor(QObject *p = Q_NULLPTR);
    void interceptRequest(QWebEngineUrlRequestInfo &info);
};

#endif // TCREQUESTINTERCEPTOR_H

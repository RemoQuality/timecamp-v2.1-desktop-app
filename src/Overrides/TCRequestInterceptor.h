#ifndef TCREQUESTINTERCEPTOR_H
#define TCREQUESTINTERCEPTOR_H

#include <QObject>
#include <QWebEngineUrlRequestInterceptor>

class TCRequestInterceptor : public QWebEngineUrlRequestInterceptor
{
Q_OBJECT

public:
    explicit TCRequestInterceptor(QObject *p = Q_NULLPTR);
    void interceptRequest(QWebEngineUrlRequestInfo &info) override;
};

#endif // TCREQUESTINTERCEPTOR_H

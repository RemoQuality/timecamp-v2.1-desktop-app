#include "TCRequestInterceptor.h"
#include "src/Settings.h"

#include <QDebug>

TCRequestInterceptor::TCRequestInterceptor(QObject *p) : QWebEngineUrlRequestInterceptor(p)
{

}

void TCRequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info)
{
    info.setHttpHeader(CONN_CUSTOM_HEADER_NAME, CONN_CUSTOM_HEADER_VALUE);
//    qDebug() << "[TCRequestInterceptor] URL: " << info.requestUrl();
    // if exactly the login page (so after logout)
    if(QString::compare(info.requestUrl().toString(), "https://www.timecamp.com/", Qt::CaseInsensitive) == 0){
        qDebug() << "[TCRequestInterceptor] URL: " << info.requestUrl().toString();
        qDebug() << "[TCRequestInterceptor] Redirect logout -> to login page";
        info.redirect(QUrl("https://www.timecamp.com/auth/login"));
    }
}

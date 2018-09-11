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

    bool isTCMarketingPage = QString::compare(info.requestUrl().toString(), MARKETING_URL, Qt::CaseInsensitive) == 0;

    if (isTCMarketingPage) {
        qDebug() << "[TCRequestInterceptor] URL: " << info.requestUrl().toString();
        qDebug() << "[TCRequestInterceptor] Redirect logout -> to login page";
        info.redirect(QUrl(LOGIN_URL));
    }
}

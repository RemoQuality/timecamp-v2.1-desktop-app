#include "TCRequestInterceptor.h"
#include "Settings.h"

TCRequestInterceptor::TCRequestInterceptor(QObject *p)
  :QWebEngineUrlRequestInterceptor(p)
{

}

void TCRequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info) {
    info.setHttpHeader(CONN_CUSTOM_HEADER_NAME, CONN_CUSTOM_HEADER_VALUE);
}

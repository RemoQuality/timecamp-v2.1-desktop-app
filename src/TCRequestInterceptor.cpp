#include "TCRequestInterceptor.h"

TCRequestInterceptor::TCRequestInterceptor(QObject *p)
  :QWebEngineUrlRequestInterceptor(p)
{

}

void TCRequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info) {
    info.setHttpHeader("X-DAPP", "2.0");
}

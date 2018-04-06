#include <QObject>
#include <QDesktopServices>

#include "TCNavigationInterceptor.h"

TCNavigationInterceptor::TCNavigationInterceptor(QWebEnginePage *parent) : QWebEnginePage(parent)
{
    target = parent;
}

bool TCNavigationInterceptor::acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame)
{
    Q_UNUSED(isMainFrame);

    auto *page = target;
    if (type == NavigationTypeLinkClicked && page) {
//        qDebug() << "External URL: " << url;
        QDesktopServices::openUrl(url);
    }

    return false;
}
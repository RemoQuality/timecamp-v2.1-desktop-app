#ifndef TIMECAMPDESKTOP_TCNAVIGATIONINTERCEPTOR_H
#define TIMECAMPDESKTOP_TCNAVIGATIONINTERCEPTOR_H

#include <QWebEnginePage>

class TCNavigationInterceptor : public QWebEnginePage
{
    QWebEnginePage *target;
public:
    explicit TCNavigationInterceptor(QWebEnginePage *parent);
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) override;
};

#endif //TIMECAMPDESKTOP_TCNAVIGATIONINTERCEPTOR_H

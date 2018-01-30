#include "AppData.h"
#include <QUrl>

AppData::AppData(QString appName, QString windowName, QString additionalInfo)
{
    this->appName = appName;
    this->windowName = windowName;
    this->additionalInfo = additionalInfo;
}

const QString &AppData::getAppName() const
{
    return appName;
}

void AppData::setAppName(const QString &appName)
{
    AppData::appName = appName;
}

const QString &AppData::getWindowName() const
{
    return windowName;
}

void AppData::setWindowName(const QString &windowName)
{
    AppData::windowName = windowName;
}
const QString &AppData::getAdditionalInfo() const
{
    return additionalInfo;
}
void AppData::setAdditionalInfo(const QString &additionalInfo)
{
    AppData::additionalInfo = additionalInfo;
}

QString AppData::getDomainFromAdditionalInfo()
{
    QUrl url(this->additionalInfo);
    return url.host();
}

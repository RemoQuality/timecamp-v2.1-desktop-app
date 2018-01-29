#include "AppData.h"


AppData::AppData(QString appName, QString windowName) {
    this->appName = appName;
    this->windowName = windowName;
}

const QString &AppData::getAppName() const {
    return appName;
}

void AppData::setAppName(const QString &appName) {
    AppData::appName = appName;
}

const QString &AppData::getWindowName() const {
    return windowName;
}

void AppData::setWindowName(const QString &windowName) {
    AppData::windowName = windowName;
}

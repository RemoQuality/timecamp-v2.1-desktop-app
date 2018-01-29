#ifndef THEGUI_APPDATA_H
#define THEGUI_APPDATA_H

#include <QtCore/QString>

class AppData {
    QString appName;
    QString windowName;
public:
    AppData(QString appName, QString windowName);

    const QString &getAppName() const;

    void setAppName(const QString &appName);

    const QString &getWindowName() const;

    void setWindowName(const QString &windowName);
};


#endif //THEGUI_APPDATA_H

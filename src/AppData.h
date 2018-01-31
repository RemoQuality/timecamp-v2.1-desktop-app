#ifndef THEGUI_APPDATA_H
#define THEGUI_APPDATA_H

#include <QString>

class AppData
{
public:
    AppData(QString appName, QString windowName, QString additionalInfo);

    const QString &getAppName() const;
    void setAppName(const QString &appName);

    const QString &getWindowName() const;
    void setWindowName(const QString &windowName);

    const QString &getAdditionalInfo() const;
    void setAdditionalInfo(const QString &additionalInfo);

    QString getDomainFromAdditionalInfo();

private:
    QString appName;
    QString windowName;
    QString additionalInfo;
};


#endif //THEGUI_APPDATA_H

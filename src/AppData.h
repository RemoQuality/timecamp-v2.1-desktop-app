#ifndef THEGUI_APPDATA_H
#define THEGUI_APPDATA_H

#include <QString>

class AppData
{
public:
    AppData();
    AppData(QString appName, QString windowName, QString additionalInfo);

    const QString &getAppName() const;
    void setAppName(const QString &appName);

    const QString &getWindowName() const;
    void setWindowName(const QString &windowName);

    const QString &getAdditionalInfo() const;
    void setAdditionalInfo(const QString &additionalInfo);

    qint64 getStart() const;
    void setStart(qint64 start);

    qint64 getEnd() const;
    void setEnd(qint64 end);

    QString getDomainFromAdditionalInfo();

private:
    QString appName;
    QString windowName;
    QString additionalInfo;
    qint64 start;
    qint64 end;
};


#endif //THEGUI_APPDATA_H

#ifndef WindowEvents_M_H
#define WindowEvents_M_H

#include <exception>
#include <iostream>
#include <string>
#include <array>
#include <memory>
#include <sstream>

#include "WindowEvents.h"
#include "Comms.h"


class WindowEvents_M : public WindowEvents {
Q_OBJECT
public:
    void static logAppName(QString appName, QString windowName, QString additionalInfo);

protected:
    void run() override; // your thread implementation goes here
    void logAppName(unsigned char *appName, unsigned char *windowName) override;

    unsigned long getIdleTime() override;

private:
    QString appTitle;
    QString processName;
    QString additionalInfo;

    void GetActiveApp();
    void GetProcWindowName();
    void GetProcNameFromPath();
    void GetAdditionalInfo();
};

#endif // WindowEvents_M_H

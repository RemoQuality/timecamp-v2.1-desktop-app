#ifndef WindowEvents_M_H
#define WindowEvents_M_H

#include <exception>
#include <iostream>
#include <string>
#include <array>
#include <memory>
#include <sstream>

#include "WindowEvents.h"


class WindowEvents_M : public WindowEvents
{
Q_OBJECT
public:
    QString GetProcWindowName(QString processName);
//    QString GetProcNameFromPath(QString processName);
    QString GetAdditionalInfo(QString processName);
    const void didActivateApp(void *anNSnotification) const;
    void *getWindowScriptObj;
    void initAppleScript();

public slots:
    void GetActiveApp(QString processName = "");

protected:
    void run() override; // your thread implementation goes here

    unsigned long getIdleTime() override;

private:
    void *workspaceWatcher;
};

#endif // WindowEvents_M_H

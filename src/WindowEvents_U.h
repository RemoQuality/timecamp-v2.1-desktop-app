#ifndef WindowEvents_U_H
#define WindowEvents_U_H

#include <exception>
#include <iostream>
#include <string>
#include <array>
#include <memory>
#include <sstream>
#include "WindowEvents.h"

class WindowEvents_U : public WindowEvents
{
protected:
    void run() override; // your thread implementation goes here
    void logAppName(unsigned char* appName, unsigned char* windowName) override;
    unsigned long getIdleTime() override;
    void logAppName(QString appName, QString windowName);

private:
    static std::string execCommand(const char* cmd);
};

#endif // WindowEvents_U_H

#ifndef WindowEvents_M_H
#define WindowEvents_M_H

#include "WindowEvents.h"
#include <exception>
#include <iostream>
#include <string>
#include <array>
#include <memory>
#include <sstream>

class WindowEvents_M : public WindowEvents
{
    Q_OBJECT

protected:
    void run() override; // your thread implementation goes here
    void logAppName(unsigned char* appName, unsigned char* windowName) override;
    unsigned long getIdleTime() override;

private:
    static std::string execCommand(const char* cmd);
};

#endif // WindowEvents_M_H

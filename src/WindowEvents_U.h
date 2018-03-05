#ifndef WindowEvents_U_H
#define WindowEvents_U_H

#include "WindowEvents.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <exception>
#include <iostream>
#include <string>
#include <array>
#include <memory>
#include <sstream>

class WindowEvents_U : public WindowEvents
{
    Q_OBJECT

protected:
    void run() override; // your thread implementation goes here
    void logAppName(unsigned char* appName, unsigned char* windowName) override;
    unsigned long getIdleTime() override;

private:
    static std::string execCommand(const char* cmd);
};

#endif // WindowEvents_U_H

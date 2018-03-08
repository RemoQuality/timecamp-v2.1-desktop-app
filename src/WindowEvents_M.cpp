#include "WindowEvents_M.h"

unsigned long WindowEvents_M::getIdleTime()
{

    return 0;
}

std::string WindowEvents_M::execCommand(const char* cmd)
{
    return "";
}

void WindowEvents_M::logAppName(unsigned char* appName, unsigned char* windowName)
{
    qInfo("%s\n", appName);
    qInfo("%s\n", windowName);
}

void WindowEvents_M::run()
{
    qInfo("thread started");

    qInfo("thread stopped");
}

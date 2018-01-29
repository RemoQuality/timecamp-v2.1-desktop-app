#ifndef WINDOWEVENTS_H
#define WINDOWEVENTS_H

#include <QThread>

class WindowEvents : public QThread
{
protected:
    virtual void run() = 0;
    virtual void logAppName(unsigned char* appName, unsigned char* windowName) = 0;
};

#endif // WINDOWEVENTS_H

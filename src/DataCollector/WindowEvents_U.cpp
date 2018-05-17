#include "WindowEvents_U.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/scrnsaver.h>

#include <src/FirefoxUtils.h>
#include <src/ChromeUtils.h>

unsigned long WindowEvents_U::getIdleTime()
{
    XScreenSaverInfo *info = XScreenSaverAllocInfo();
    Display *display;

    display = XOpenDisplay(NULL);

    if (display == nullptr) {
        qInfo() << "[WindowEvents_U::getIdleTime] GetIdleTime failed";
    } else {
        XScreenSaverQueryInfo(display, DefaultRootWindow(display), info);
        XCloseDisplay(display);
        return info->idle;
    }

    return 0;
}

std::string WindowEvents_U::execCommand(const char *cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr) {
            result += buffer.data();
        }
    }
    return result;
}

void WindowEvents_U::logAppName(QString appName, QString windowName)
{
    AppData *app;
    QString additionalInfo = "";

    if (appName == "firefox") {
        app = WindowEvents::logAppName(appName, windowName, appName); // set additionalInfo to appName for now
        additionalInfo = getCurrentURLFromFirefox(); // somewhat unreliable - data is usually a few seconds late into the file
    } else if (appName == "chrome") {
        app = WindowEvents::logAppName(appName, windowName, appName); // same as above, just to skip the "Internet" checker
        additionalInfo = getCurrentURLFromChrome(windowName); // somewhat unreliable - might not get the URL
    }

    if (additionalInfo != "") {
        app->setAdditionalInfo(additionalInfo); // after we get the URL, update additionalInfo
    } else {
        WindowEvents::logAppName(appName, windowName, additionalInfo);
    }
}

void WindowEvents_U::run()
{
    qInfo("thread started");
    Display *display;
    Window root;
    XEvent event;

    display = XOpenDisplay(nullptr);

    if (display == nullptr) {
        qInfo("[WindowEvents_U] Error: XOpenDisplay");
        return;
    }

    Atom NET_ACTIVE_WINDOW = XInternAtom(display, "_NET_ACTIVE_WINDOW", false);
    Atom NET_WM_NAME = XInternAtom(display, "_NET_WM_NAME", false);  // UTF-8
    Atom WM_NAME = XInternAtom(display, "WM_NAME", false);           // Legacy encoding
    Atom NET_WM_PID = XInternAtom(display, "_NET_WM_PID", false);           // Legacy encoding

    root = XDefaultRootWindow(display);

    XSelectInput(display, root, PropertyChangeMask);

    char *last_name;
    char *curr_name;

    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long bytes;
    long *data;
    unsigned char *window_name;
    unsigned char *pid;
    std::string app_name;
    int status;
    long xwindowid_old;
    long xwindowid_curr;

    XTextProperty *text_prop_return;

    while (!QThread::currentThread()->isInterruptionRequested()) {
        XNextEvent(display, &event);
        if (event.type == PropertyNotify) {
            if (event.xproperty.atom == NET_ACTIVE_WINDOW || event.xproperty.atom == NET_WM_NAME || event.xproperty.atom == WM_NAME) {
                status = XGetWindowProperty(
                    display,
                    root,
                    NET_ACTIVE_WINDOW,
                    0,
                    (~0L),
                    False,
                    AnyPropertyType,
                    &actual_type,
                    &actual_format,
                    &nitems,
                    &bytes,
                    (unsigned char **) &data);

                if (status != Success) {
                    qInfo("[WindowEvents_U] Error of status = %d\n", status);
                    continue;
                }

                if (xwindowid_curr != data[0] && data[0] != 0) {
                    XSelectInput(display, xwindowid_old, NoEventMask); // don't fetch events from old window

                    xwindowid_old = xwindowid_curr; // what was current is now old
                    xwindowid_curr = data[0];       // set new "current"

                    XSelectInput(display, xwindowid_curr, PropertyChangeMask);
                }

                status = XGetWindowProperty(
                    display,
                    xwindowid_curr,
                    NET_WM_NAME,
                    0,
                    (~0L),
                    False,
                    AnyPropertyType,
                    &actual_type,
                    &actual_format,
                    &nitems,
                    &bytes,
                    &window_name);

                if (status != Success) {
                    qInfo("[WindowEvents_U] Error of status = %d\n", status);
                    continue;
                }

                status = XGetWindowProperty(
                    display,
                    xwindowid_curr,
                    NET_WM_PID,
                    0,
                    (~0L),
                    False,
                    AnyPropertyType,
                    &actual_type,
                    &actual_format,
                    &nitems,
                    &bytes,
                    &pid);

                if (status != Success) {
                    qInfo("[WindowEvents_U] Error of status = %d\n", status);
                    continue;
                }
                if(pid == nullptr){
                    qInfo("[WindowEvents_U] Error: pid was NULL");
                    continue;
                }

                auto *longarr = reinterpret_cast<long *>(pid);
                long longpid = longarr[0];

                std::string command = "";
                command += "ps ";
                command += " -o comm= ";
                command += QString::number(longpid).toStdString();

//                qInfo() << QString::fromStdString(command);

                app_name = execCommand(command.c_str());

                logAppName(QString::fromStdString(app_name), QString::fromUtf8((char*)window_name));
            }
        }
    }

    XCloseDisplay(display);
    qInfo("thread stopped");
}

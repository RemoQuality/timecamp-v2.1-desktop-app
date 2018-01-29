#include "WindowEvents_U.h"

std::string WindowEvents_U::execCommand(const char* cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

void WindowEvents_U::logAppName(unsigned char* data)
{
    qInfo("%s\n", data);
}

void WindowEvents_U::run()
{
    qInfo("thread started");
    Display *display;
    Window root;
    XEvent event;

    display = XOpenDisplay(NULL);

    if (display == NULL) {
        printf("Error: XOpenDisplay");
        return;
    }

    Atom NET_ACTIVE_WINDOW = XInternAtom(display, "_NET_ACTIVE_WINDOW", false);
    Atom NET_WM_NAME = XInternAtom(display, "_NET_WM_NAME", false);  // UTF-8
    Atom WM_NAME = XInternAtom(display, "WM_NAME", false);           // Legacy encoding

    root = XDefaultRootWindow(display);

    XSelectInput(display, root, PropertyChangeMask);

    char *last_name;
    char *curr_name;

    Atom     actual_type;
    int      actual_format;
    unsigned long     nitems;
    unsigned long     bytes;
    long     *data;
    unsigned char     *name_data;
    int      status;
    long xwindowid_old;
    long xwindowid_curr;

    XTextProperty *text_prop_return;

    while (!QThread::currentThread()->isInterruptionRequested()) {
        XNextEvent(display, &event);
        if(event.type == PropertyNotify) {
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
                    (unsigned char**)&data);

                if (status != Success) {
                    fprintf(stderr, "status = %d\n", status);
                    exit(1);
                }

                if(xwindowid_curr != data[0] && data[0] != 0){
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
                    &name_data);

                if (status != Success) {
                    fprintf(stderr, "status = %d\n", status);
                    exit(1);
                }
                logAppName(name_data);
            }
        }
    }

    XCloseDisplay(display);
    qInfo("thread stopped");
}

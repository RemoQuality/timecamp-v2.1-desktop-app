#ifndef WINDOWEVENTS_W_H
#define WINDOWEVENTS_W_H

#include "WindowEvents.h"
#include <Windows.h>
#include <oleacc.h>
#include <OleAuto.h>
#include <objbase.h>
#include <tchar.h>
#include <Psapi.h>
#include <comutil.h>

#include "Comms.h"
#include "AppData.h"

typedef struct {
    DWORD ownerpid;
    DWORD childpid;
} windowinfo;

class WindowEvents_W : public WindowEvents
{
public:
    void static logAppName(QString appName, QString windowName);
    static double getWindowsVersion();

protected:
    void run() override; // your thread implementation goes here
    void logAppName(unsigned char* appName, unsigned char* windowName) override;

private:
    HWINEVENTHOOK wname_hook;
    HWINEVENTHOOK g_hook;
    MSG Komunikat;

    static void HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
                        LONG idObject, LONG idChild,
                        DWORD dwEventThread, DWORD dwmsEventTime);
    static void HandleWinNameEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
                        LONG idObject, LONG idChild,
                        DWORD dwEventThread, DWORD dwmsEventTime);

    static BOOL EnumChildAppHostWindowsCallback(HWND hWnd, LPARAM lp);
    static void GetProcessName(HWND hWnd, TCHAR *procName);
    static void ParseProcessName(HANDLE hProcess, TCHAR *processName);

    void InitializeWindowsHook(HWINEVENTHOOK g_hook, HWINEVENTHOOK wname_hook);
    void ShutdownWindowsHook(HWINEVENTHOOK g_hook, HWINEVENTHOOK wname_hook);
};

#endif // WINDOWEVENTS_W_H

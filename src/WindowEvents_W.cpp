#include "WindowEvents_W.h"
#include "ControlIterator/UIAControlIterator.h"

void WindowEvents_W::logAppName(unsigned char* appName, unsigned char* windowName)
{
    //qInfo("APP: %s | %s \n", appName, windowName);
}

void WindowEvents_W::logAppName(QString appName, QString windowName)
{
    //qInfo("APP: %s | %s \n", appName.toLatin1().constData(), windowName.toLatin1().constData());
    appName.replace(".exe", "");
    AppData *app = new AppData(appName, windowName);
    Comms::instance().saveApp(app);
}

void WindowEvents_W::run()
{
    qInfo("thread started");

    InitializeWindowsHook(g_hook, wname_hook);

    // Pętla komunikatów WinAPI

    while (!QThread::currentThread()->isInterruptionRequested() && GetMessage(&Komunikat, nullptr, 0, 0)) {
        TranslateMessage(&Komunikat);
        DispatchMessage(&Komunikat);
    }

    ShutdownWindowsHook(g_hook, wname_hook);

    qInfo("thread stopped");
}

BOOL WindowEvents_W::EnumChildAppHostWindowsCallback(HWND hWnd, LPARAM lp)
{
    windowinfo *info = (windowinfo *) lp;
    DWORD pid = 0;
    GetWindowThreadProcessId(hWnd, &pid);
    if (pid != info->ownerpid) info->childpid = pid;
    return TRUE;
}

void WindowEvents_W::HandleWinNameEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
                             LONG idObject, LONG idChild,
                             DWORD dwEventThread, DWORD dwmsEventTime)
{
    IAccessible *pAcc = nullptr;
    VARIANT varChild;
    HWND foreground = GetForegroundWindow();
    if (hwnd != nullptr && foreground != nullptr && foreground == hwnd) {
        HRESULT hr = AccessibleObjectFromEvent(hwnd, idObject, idChild, &pAcc, &varChild);

        if ((hr == S_OK) && (pAcc != nullptr)) {

            if(idObject == OBJID_TITLEBAR || idObject == OBJID_WINDOW){

                BSTR bstrName; // window title (website name, etc)
                pAcc->get_accName(varChild, &bstrName);

                TCHAR procName[255];
                GetProcessName(hwnd, procName);

                 QString procNameNorm;
    #ifdef _UNICODE
                 std::wstring tempName(&procName[0]);
                 procNameNorm = QString::fromStdWString(tempName);
    #else
                 procNameNorm = QString::fromLatin1(procName);
    #endif

                 QString windowName2((QChar*)bstrName, SysStringLen(bstrName));

    //            TCHAR windowName[255];
    //            MultiByteToWideChar(CP_UTF8, 0, bstrName, SysStringLen(bstrName), windowName, 255);
                char *windowName;
                windowName = _com_util::ConvertBSTRToString(bstrName);

                if (SysStringLen(bstrName) > 0) {
                    WindowEvents_W::logAppName(procNameNorm, windowName2);
                }

                SysFreeString(bstrName);
                pAcc->Release();
            }
        }
    }

}

// Callback function that handles events.
void WindowEvents_W::HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
                             LONG idObject, LONG idChild,
                             DWORD dwEventThread, DWORD dwmsEventTime)
{
    IAccessible *pAcc = nullptr;
    VARIANT varChild;
    if (hwnd != nullptr) {
        HRESULT hr = AccessibleObjectFromEvent(hwnd, idObject, idChild, &pAcc, &varChild);

        if ((hr == S_OK) && (pAcc != nullptr)) {

            DWORD dwProcessId;
            GetWindowThreadProcessId(hwnd, &dwProcessId);

            BSTR bstrName; // window title (website name, etc)
            pAcc->get_accName(varChild, &bstrName);

            TCHAR procName[255];
            GetProcessName(hwnd, procName);

             QString procNameNorm;
#ifdef _UNICODE
             std::wstring tempName(&procName[0]);
             procNameNorm = QString::fromStdWString(tempName);
#else
             procNameNorm = QString::fromLatin1(procName);
#endif

             QString windowName2((QChar*)bstrName, SysStringLen(bstrName));

//            TCHAR windowName[255];
//            MultiByteToWideChar(CP_UTF8, 0, bstrName, SysStringLen(bstrName), windowName, 255);
            char *windowName;
            windowName = _com_util::ConvertBSTRToString(bstrName);

            if (SysStringLen(bstrName) > 0) {
                 WindowEvents_W::logAppName(procNameNorm, windowName2);
            }

            SysFreeString(bstrName);
            pAcc->Release();
        }
    }
}

void WindowEvents_W::GetProcessName(HWND hWnd, TCHAR *procName)
{
    DWORD dwProcessId;
    HANDLE hProcess;
    HANDLE active_process;

    GetWindowThreadProcessId(hWnd, &dwProcessId);

    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, false, dwProcessId);
    if (hProcess == nullptr) {
        if (WindowEvents_W::getWindowsVersion() > 5) {
            hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, dwProcessId);
            if (hProcess == nullptr) {
                procName = (TCHAR *) _T("");
            }
        }
    }

    WindowEvents_W::ParseProcessName(hProcess, procName);

    if (memcmp(procName, TEXT("ApplicationFrameHost.exe"), sizeof(procName)) == 0) { // we need to go deeper
        windowinfo info = {0, 0}; // set owner and child to zeros

        GetWindowThreadProcessId(hWnd, &info.ownerpid);
        info.childpid = info.ownerpid;

        EnumChildWindows(hWnd, WindowEvents_W::EnumChildAppHostWindowsCallback,
                         (LPARAM) &info); // go through all windows and find the right child

        active_process = OpenProcess(PROCESS_QUERY_INFORMATION, false, info.childpid);
        if (active_process == nullptr) {
            if (getWindowsVersion() > 5) {
                active_process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, info.childpid);
                if (active_process == nullptr) {
                    procName = (TCHAR *) _T("explorer000");
                }
            }
        }

        WindowEvents_W::ParseProcessName(active_process, procName);

        CloseHandle(active_process);
    }

    CloseHandle(hProcess);
}

double WindowEvents_W::getWindowsVersion()
{
    OSVERSIONINFO osvi;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osvi);

    return osvi.dwMajorVersion + 0.1 * osvi.dwMinorVersion;
}

void WindowEvents_W::ParseProcessName(HANDLE hProcess, TCHAR *processName)
{
    TCHAR procChar[MAX_PATH];
    DWORD namelen = GetProcessImageFileName(hProcess, procChar, sizeof(procChar) / sizeof(*procChar));
    if (0 == namelen) {
        processName = (TCHAR *) _T("");
        return;
    }

    std::basic_string<TCHAR> procName = procChar;
    size_t lastPath = procName.find_last_of(_T('\\'));
    size_t pos = lastPath + 1;
    size_t len = procName.length() - lastPath - 1;
    procName = procName.substr(pos, len);

    std::size_t length = procName.copy(processName, procName.length(), 0);
    processName[length] = '\0'; // null terminate!
}

// Initializes COM and sets up the event hook.
void WindowEvents_W::InitializeWindowsHook(HWINEVENTHOOK g_hook, HWINEVENTHOOK wname_hook)
{
    CoInitialize(nullptr);
    g_hook = SetWinEventHook(
            EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND,   // Range of events
            nullptr,                                            // Handle to DLL
            WindowEvents_W::HandleWinEvent,                                     // The callback
            0, 0,                                               // Process and thread IDs of interest (0 = all)
            WINEVENT_OUTOFCONTEXT);

    wname_hook = SetWinEventHook(
            EVENT_OBJECT_NAMECHANGE, EVENT_OBJECT_NAMECHANGE,   // Range of events
            nullptr,                                            // Handle to DLL
            WindowEvents_W::HandleWinNameEvent,                                 // The callback
            0, 0,                                     // Process and thread IDs of interest (0 = all)
            WINEVENT_OUTOFCONTEXT);
}

// Unhooks the event and shuts down COM.
void WindowEvents_W::ShutdownWindowsHook(HWINEVENTHOOK g_hook, HWINEVENTHOOK wname_hook)
{
    UnhookWinEvent(wname_hook);
    UnhookWinEvent(g_hook);
    CoUninitialize();
}

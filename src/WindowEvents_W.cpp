#include <src/ControlIterator/AccControlIterator.h>
#include "WindowEvents_W.h"
#include "ControlIterator/UIAControlIterator.h"
#include "FirefoxUtils.h"
#include <QElapsedTimer>

void WindowEvents_W::logAppName(unsigned char* appName, unsigned char* windowName)
{
    //qInfo("APP: %s | %s \n", appName, windowName);
}

void WindowEvents_W::logAppName(QString appName, QString windowName, HWND passedHwnd)
{
    qInfo("APP: %s | %s \n", appName.toLatin1().constData(), windowName.toLatin1().constData());
    appName.replace(".exe", "");
    WindowDetails *details = new WindowDetails();
    QString additionalInfo = details->GetAdditionalInfo(appName, passedHwnd);
    AppData *app = new AppData(appName, windowName, additionalInfo);
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
                    WindowEvents_W::logAppName(procNameNorm, windowName2, hwnd);
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

// broken
//            TCHAR windowName[255];
//            MultiByteToWideChar(CP_UTF8, 0, bstrName, SysStringLen(bstrName), windowName, 255);

// was working, but not Qt
//            char *windowName;
//            windowName = _com_util::ConvertBSTRToString(bstrName);

            if (SysStringLen(bstrName) > 0) {
                 WindowEvents_W::logAppName(procNameNorm, windowName2, hwnd);
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
        if (getWindowsVersion() > 5) {
            hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, dwProcessId);
            if (hProcess == nullptr) {
                procName = (TCHAR *) _T("");
            }
        }
    }

    ParseProcessName(hProcess, procName);

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
                    procName = (TCHAR *) _T("");
                }
            }
        }

        ParseProcessName(active_process, procName);

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

bool WindowDetails::startsWithGoodProtocol(QString checkedStr)
{
    if (checkedStr.midRef(0, 7) == "http://" || checkedStr.midRef(0, 8) == "https://" || checkedStr.midRef(0, 6) == "ftp://" || checkedStr.midRef(0, 7) == "file://") {
        return true;
    }
    return false;
}

bool WindowDetails::standardAccCallback(IControlItem *node, void *userData)
{
    QString *pStr = (QString *) userData;
    QString value = QString::fromStdWString(node->getValue());


    if (startsWithGoodProtocol(value)) {
        qDebug() << "[VAL] " << value;
        if (*pStr == "" || *pStr == "0" || *pStr == "<unknown>") {
            *pStr = value;
            return false;
        }
    }

    return true;
}

bool WindowDetails::chromeAccCallback(IControlItem *node, void *userData)
{
    QString value = QString::fromStdWString(node->getValue());
    if (value != "" && value != "0" && value != "<unknown>") {
//        qDebug() << "[WForegroundApp::chromeAccCallback] Got IControlItem node value = " << value;
        qDebug() << "[VAL] " << value;

        if (node->parent->getRole() == ROLE_SYSTEM_GROUPING && value.contains(URL_REGEX)) {
//            qDebug("[WForegroundApp::chromeAccCallback] It is a valid URL, so we return it.");
            if(!startsWithGoodProtocol(value)){
                value = "http://" + value; // prepend http to it to fix URLs!
            }
            QString *pStr = (QString *) userData;
            *pStr = value;
            return false;
        }
    }
    return true;
}

bool WindowDetails::operaAccCallback(IControlItem *node, void *userData)
{
    QString *pStr = (QString *) userData;
    QString value = QString::fromStdWString(node->getValue());
    if (value != "" && value != "0" && value != "<unknown>") {
//        qDebug() << "[WForegroundApp::operaAccCallback] Got IControlItem node value = " << value;
        qDebug() << "[VAL] " << value;

        if (value.contains(URL_REGEX)) {
//            qDebug("[WForegroundApp::operaAccCallback] It is a valid URL, so we return it.");
            if(!startsWithGoodProtocol(value)){
                value = "http://" + value; // prepend http to it to fix URLs!
            }
            *pStr = value;
            return false;
        }
    }

    return true;
}

WindowDetails::WindowDetails()
{
    URL_REGEX_STR = QString("^") +
        QString("(?:") +
        // protocol identifier
        QString("(?:(?:https?|ftp)://)?") +
        // user:pass authentication
        QString("(?:\\S+(?::\\S*)?@)?") +
        QString("(?:") +
        // IP address exclusion
        // private & local networks
        // IP address dotted notation octets
        // excludes loopback network 0.0.0.0
        // excludes reserved space >= 224.0.0.0
        // excludes network & broacast addresses
        // (first & last IP address of each class)
        QString("(?:[1-9]\\d?|1\\d\\d|2[01]\\d|22[0-3])") +
        QString("(?:\\.(?:1?\\d{1,2}|2[0-4]\\d|25[0-5])){2}") +
        QString("(?:\\.(?:[1-9]\\d?|1\\d\\d|2[0-4]\\d|25[0-4]))") +
        QString("|") +
        // host name
        QString("(?:(?:[a-z\\u00a1-\\uffff0-9]+-?)*[a-z\\u00a1-\\uffff0-9]+)") +
        // domain name
        QString("(?:") +
        QString("(?:\\.(?:[a-z\\u00a1-\\uffff0-9]+-?)*[a-z\\u00a1-\\uffff0-9]+)*") +
        // TLD identifier
        QString("(?:\\.(?:[a-z\\u00a1-\\uffff]{2,}))") +
        //Domain exceptions for single segment domains (without port numbers)
        QString(")|(localhost)|(crm)|(replace_me)") +
        QString(")") +
        // port number
        QString("(?::\\d{2,5})?") +
        // resource path
        QString("(?:/[^\\s]*)?") +
        QString(")") +
        QString("|(?:file://.*)") +
        QString("$");

    URL_REGEX = QRegExp(URL_REGEX_STR);
}

QString WindowDetails::GetAdditionalInfo(QString processName, HWND passedHwnd)
{
    processName = processName.toLower();
    if (passedHwnd == NULL) {
        currenthwnd = GetForegroundWindow();
    } else {
        currenthwnd = passedHwnd;
    }

    bool browser = false;
    auto pointerMagic = &WindowDetails::standardAccCallback;

    // iexplore gets only first tab (!)
    // all others were not checked
    if (processName.toLower().contains(QRegExp("iexplore|mosaic|maxthon|safari"))) {
        pointerMagic = &WindowDetails::standardAccCallback;
        browser = true;
    }

    /*
        chromium / blink based:
            Chromium (for Windows): chrome.exe - works
            Google Chrome: chrome.exe - works
            SRWare Iron: chrome.exe - works
            Opera: opera.exe - works
            Epic Privacy Browser: epic.exe - works
            all other - not checked
     */
    if (processName.toLower().contains(QRegExp("chrome|epic|opera|cent|slimjet|sleipnir|silk|blisk|yandex|iron"))) {
        pointerMagic = &WindowDetails::chromeAccCallback;
        browser = true;
    }

    /*
        ALL, so:
            MS Edge: weird processes - somewhat works, but inserts random pages sometimes
            Tor: firefox.exe - not checked yet

        "broken" chromium based:
            Yandex Browser: browser.exe - (gets only website title via chromeCb) either cut after space, or use regex result?
            Vivaldi: vivaldi.exe - STILL broken; nothing works at all
            Brave: brave.exe - STILL broken; nothing works at all
            UC Browser: UCBrowser.exe -STILL broken; nothing works at all
     */
    if (processName.toLower().contains(QRegExp("microsoftedge|netscp6|mozilla|netscape|vivaldi|brave|ucbrowser|browser"))) {
        pointerMagic = &WindowDetails::operaAccCallback;
        browser = true;
    }

    if (browser) {
        QString res("");
        QElapsedTimer timer;
        timer.start();
        AccControlIterator iterator;
        iterator.iterate(currenthwnd, this, pointerMagic, (void *) &res, true);

        qInfo() << "[ACC] " << res;

        if (res == "" && WindowEvents_W::getWindowsVersion() <= 6.0) {
            UIAControlIterator iterator2;
            iterator2.iterate(currenthwnd, this, pointerMagic, (void *) &res, true);
            qInfo() << "[UIA] " << res;// << "\r\n";
        }

        QUrl url(res);
        QString host = url.host();
        qInfo() << "[HOST]" << host << "(" << timer.elapsed() << ")" << "ms" << "\r\n";
        return res;
    }

    if (processName.toLower().contains(QRegExp("firefox"))) {
        return getCurrentURLFromFirefox();
    }

    return "";
}
const QRegExp &WindowDetails::getURL_REGEX() const
{
    return URL_REGEX;
}
void WindowDetails::setURL_REGEX(const QRegExp &URL_REGEX)
{
    WindowDetails::URL_REGEX = URL_REGEX;
}

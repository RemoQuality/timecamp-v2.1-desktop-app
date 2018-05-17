#include "WindowEvents_W.h"
#include "src/ControlIterator/AccControlIterator.h"
#include "src/ControlIterator/UIAControlIterator.h"
#include <QElapsedTimer>
#include <QUrl>

unsigned long WindowEvents_W::getIdleTime()
{
    LASTINPUTINFO lpi;
    lpi.cbSize = sizeof(LASTINPUTINFO);

    if (GetLastInputInfo(&lpi)) {
        return (GetTickCount() - lpi.dwTime); //pobierz czas bezczynnosci po power resume
    } else {
        qDebug() << "[WindowEvents_W::getIdleTime] GetIdleTime failed";
    }

    return 0;
}

void WindowEvents_W::logAppName(QString appName, QString windowName, HWND passedHwnd)
{
    appName = appName.replace(".exe", "");

    AppData *app;
    QString additionalInfo = "";

    if (WindowDetails::instance().isBrowser(appName)) {
        app = WindowEvents::logAppName(appName, windowName, appName); // set additionalInfo to appName for now
        additionalInfo = WindowDetails::instance().GetInfoFromBrowser(passedHwnd); // get real URL
    } else if (appName.toLower().contains(QRegExp("firefox"))) {
        app = WindowEvents::logAppName(appName, windowName, appName); // same like above, just to skip the "Internet" checker
        additionalInfo = WindowDetails::instance().GetInfoFromFirefox(passedHwnd); // get real URL from Firefox
    }

    if(additionalInfo != "") {
        app->setAdditionalInfo(additionalInfo); // after we get the URL, update additionalInfo
    } else {
        WindowEvents::logAppName(appName, windowName, additionalInfo);
    }
}

void WindowEvents_W::run()
{
    qInfo("thread started");

    InitializeWindowsHook(appChangeEventHook, appNameChangeEventHook);

    // WinAPI loop

    while (!QThread::currentThread()->isInterruptionRequested() && GetMessage(&winApiMsg, nullptr, 0, 0)) {
        TranslateMessage(&winApiMsg);
        DispatchMessage(&winApiMsg);
    }

    ShutdownWindowsHook(appChangeEventHook, appNameChangeEventHook);

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

void WindowEvents_W::HandleWinNameEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG idObject, LONG idChild,
                                        DWORD dwEventThread, DWORD dwmsEventTime)
{
    IAccessible *pAcc = nullptr;
    VARIANT varChild;
    HWND foreground = GetForegroundWindow();
    if (hwnd != nullptr && foreground != nullptr && foreground == hwnd) {
        HRESULT hr = AccessibleObjectFromEvent(hwnd, idObject, idChild, &pAcc, &varChild);

        if ((hr == S_OK) && (pAcc != nullptr)) {

            if (idObject == OBJID_TITLEBAR || idObject == OBJID_WINDOW) {

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

                QString windowName2((QChar *) bstrName, SysStringLen(bstrName));

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
void WindowEvents_W::HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG idObject, LONG idChild,
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

            QString windowName2((QChar *) bstrName, SysStringLen(bstrName));

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

        EnumChildWindows(hWnd, reinterpret_cast<WNDENUMPROC>(WindowEvents_W::EnumChildAppHostWindowsCallback), (LPARAM) & info); // go through all windows and find the right child

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
void WindowEvents_W::InitializeWindowsHook(HWINEVENTHOOK appChangeEventHook, HWINEVENTHOOK appNameChangeEventHook)
{
    CoInitialize(nullptr);
    appChangeEventHook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND,   // Range of events
                             nullptr,                                            // Handle to DLL
                             reinterpret_cast<WINEVENTPROC>(WindowEvents_W::HandleWinEvent),                                     // The callback
                             0, 0,                                               // Process and thread IDs of interest (0 = all)
                             WINEVENT_OUTOFCONTEXT);

    appNameChangeEventHook = SetWinEventHook(EVENT_OBJECT_NAMECHANGE, EVENT_OBJECT_NAMECHANGE,   // Range of events
                                 nullptr,                                            // Handle to DLL
                                 reinterpret_cast<WINEVENTPROC>(WindowEvents_W::HandleWinNameEvent),                                 // The callback
                                 0, 0,                                     // Process and thread IDs of interest (0 = all)
                                 WINEVENT_OUTOFCONTEXT);
}

// Unhooks the event and shuts down COM.
void WindowEvents_W::ShutdownWindowsHook(HWINEVENTHOOK appChangeEventHook, HWINEVENTHOOK appNameChangeEventHook)
{
    UnhookWinEvent(appNameChangeEventHook);
    UnhookWinEvent(appChangeEventHook);
    CoUninitialize();
}

bool WindowDetails::startsWithGoodProtocol(QString checkedStr)
{
    if (checkedStr.midRef(0, 7) == QLatin1String("http://") || checkedStr.midRef(0, 8) == QLatin1String("https://") || checkedStr.midRef(0, 6) == QLatin1String("ftp://") || checkedStr.midRef(0, 7) == QLatin1String("file://")) {
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
        if (pStr->isEmpty() || *pStr == QLatin1String("0") || *pStr == QLatin1String("<unknown>")) {
            *pStr = value;
            return false;
        }
    }

    return true;
}

bool WindowDetails::chromeAccCallback(IControlItem *node, void *userData)
{
    QString value = QString::fromStdWString(node->getValue());
    if (!value.isEmpty() && value != QLatin1String("0") && value != QLatin1String("<unknown>")) {
//        qDebug() << "[WForegroundApp::chromeAccCallback] Got IControlItem node value = " << value;
        qDebug() << "[VAL] " << value;

        if (node->parent->getRole() == ROLE_SYSTEM_GROUPING && value.contains(URL_REGEX)) {
//            qDebug("[WForegroundApp::chromeAccCallback] It is a valid URL, so we return it.");
            if (!startsWithGoodProtocol(value)) {
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
    if (!value.isEmpty() && value != QLatin1String("0") && value != QLatin1String("<unknown>")) {
//        qDebug() << "[WForegroundApp::operaAccCallback] Got IControlItem node value = " << value;
        qDebug() << "[VAL] " << value;

        if (value.contains(URL_REGEX)) {
//            qDebug("[WForegroundApp::operaAccCallback] It is a valid URL, so we return it.");
            if (!startsWithGoodProtocol(value)) {
                value = "http://" + value; // prepend http to it to fix URLs!
            }
            *pStr = value;
            return false;
        }
    }

    return true;
}

WindowDetails &WindowDetails::instance()
{
    static WindowDetails _instance;
    return _instance;
}

WindowDetails::WindowDetails()
{
    URL_REGEX_STR = QString("^") + QString("(?:") +
                    // protocol identifier
                    QString("(?:(?:https?|ftp)://)?") +
                    // user:pass authentication
                    QString("(?:\\S+(?::\\S*)?@)?") + QString("(?:") +
                    // IP address exclusion
                    // private & local networks
                    // IP address dotted notation octets
                    // excludes loopback network 0.0.0.0
                    // excludes reserved space >= 224.0.0.0
                    // excludes network & broacast addresses
                    // (first & last IP address of each class)
                    QString("(?:[1-9]\\d?|1\\d\\d|2[01]\\d|22[0-3])") + QString("(?:\\.(?:1?\\d{1,2}|2[0-4]\\d|25[0-5])){2}") + QString("(?:\\.(?:[1-9]\\d?|1\\d\\d|2[0-4]\\d|25[0-4]))") + QString("|") +
                    // host name
                    QString("(?:(?:[a-z\\u00a1-\\uffff0-9]+-?)*[a-z\\u00a1-\\uffff0-9]+)") +
                    // domain name
                    QString("(?:") + QString("(?:\\.(?:[a-z\\u00a1-\\uffff0-9]+-?)*[a-z\\u00a1-\\uffff0-9]+)*") +
                    // TLD identifier
                    QString("(?:\\.(?:[a-z\\u00a1-\\uffff]{2,}))") +
                    //Domain exceptions for single segment domains (without port numbers)
                    QString(")|(localhost)|(crm)|(replace_me)") + QString(")") +
                    // port number
                    QString("(?::\\d{2,5})?") +
                    // resource path
                    QString("(?:/[^\\s]*)?") + QString(")") + QString("$");

    URL_REGEX = QRegExp(URL_REGEX_STR);
}

bool WindowDetails::isBrowser(QString processName)
{
    // iexplore gets only first tab (!)
    // all others were not checked
    if (processName.toLower().contains(QRegExp("iexplore|mosaic|maxthon|safari"))) {
        pointerMagic = &WindowDetails::standardAccCallback;
        return true;
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
        return true;
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
        return true;
    }

    return false;
}

QString WindowDetails::GetInfoFromFirefox(HWND passedHwnd)
{
    if (passedHwnd == NULL) {
        currenthwnd = GetForegroundWindow();
    } else {
        currenthwnd = passedHwnd;
    }

    QElapsedTimer timer;
    timer.start();
    QString res = QString::fromStdWString(FirefoxURL::GetFirefoxURL(currenthwnd));

    qDebug() << "[FX_W]" << res;
    QUrl url(res);
    QString host = url.host();
    qDebug() << "[HOST]" << host << "(" << timer.elapsed() << ")" << "ms" << "\r\n";
    return res; // we do [HOST] here for debug only, but we save full URL to DB
}

QString WindowDetails::GetInfoFromBrowser(HWND passedHwnd)
{
    if (passedHwnd == NULL) {
        currenthwnd = GetForegroundWindow();
    } else {
        currenthwnd = passedHwnd;
    }

    QString res("");
    QElapsedTimer timer;
    timer.start();
    AccControlIterator iterator;
    iterator.iterate(currenthwnd, this, pointerMagic, (void *) &res, true);

    qDebug() << "[ACC] " << res;

    if (res == "" && WindowEvents_W::getWindowsVersion() <= 6.0) {
        UIAControlIterator iterator2;
        iterator2.iterate(currenthwnd, this, pointerMagic, (void *) &res, true);
        qDebug() << "[UIA] " << res;// << "\r\n";
    }

    QUrl url(res);
    QString host = url.host();
    qDebug() << "[HOST]" << host << "(" << timer.elapsed() << ")" << "ms" << "\r\n";
    return res;
}

const QRegExp &WindowDetails::getURL_REGEX() const
{
    return URL_REGEX;
}
void WindowDetails::setURL_REGEX(const QRegExp &URL_REGEX)
{
    WindowDetails::URL_REGEX = URL_REGEX;
}

HRESULT FirefoxURL::GetControlCondition(IUIAutomation *automation, const long controlType,
                                        IUIAutomationCondition **controlCondition)
{
    VARIANT propVar;
    propVar.vt = VT_I4;
    propVar.lVal = controlType;
    return automation->CreatePropertyCondition(UIA_ControlTypePropertyId, propVar, controlCondition);
}

std::wstring FirefoxURL::GetFirefoxURL(HWND hwnd)
{
    BSTR url;
    std::wstring returnedError = L"";
    bool failedBit = false;

    IUIAutomation *_automation;
    HRESULT hr = CoInitialize(NULL);

    if (!FAILED(hr)) {
        hr = CoCreateInstance(__uuidof(CUIAutomation), NULL, CLSCTX_INPROC_SERVER, __uuidof(IUIAutomation), (void **) &_automation);
        if (!FAILED(hr)) {
            IUIAutomationElement *firefoxElement = NULL;
            hr = _automation->ElementFromHandle(hwnd, &firefoxElement);
            if (!FAILED(hr)) {
                IUIAutomationCondition *toolbarCondition;
                hr = GetControlCondition(_automation, UIA_ToolBarControlTypeId, &toolbarCondition);
                if (!FAILED(hr)) {
                    IUIAutomationElementArray *toolbars = NULL;
                    hr = firefoxElement->FindAll(TreeScope_Children, toolbarCondition, &toolbars);
                    if (!FAILED(hr)) {
                        int length = 0;
                        hr = toolbars->get_Length(&length);
                        if (!FAILED(hr)) {
                            if (length >= 3) {
                                IUIAutomationElement *toolbarElement = NULL;
                                hr = toolbars->GetElement(2, &toolbarElement);
                                if (!FAILED(hr)) {
                                    IUIAutomationCondition *comboCondition;
                                    hr = GetControlCondition(_automation, UIA_ComboBoxControlTypeId, &comboCondition);
                                    if (!FAILED(hr)) {
                                        IUIAutomationElement *comboElement = NULL;
                                        hr = toolbarElement->FindFirst(TreeScope_Children, comboCondition, &comboElement);
                                        if (!FAILED(hr)) {
                                            IUIAutomationCondition *editCondition;
                                            VARIANT propVar;
                                            propVar.vt = VT_I4;
                                            propVar.lVal = UIA_EditControlTypeId;
                                            hr = _automation->CreatePropertyCondition(UIA_ControlTypePropertyId, propVar, &editCondition);
                                            if (!FAILED(hr)) {
                                                IUIAutomationElement *urlElement = NULL;
                                                hr = comboElement->FindFirst(TreeScope_Children, editCondition, &urlElement);
                                                if (!FAILED(hr)) {
                                                    IUnknown *patternInter = NULL;
                                                    hr = urlElement->GetCurrentPattern(UIA_ValuePatternId, &patternInter);
                                                    if (!FAILED(hr)) {
                                                        IUIAutomationValuePattern *valuePattern = NULL;
                                                        hr = patternInter->QueryInterface(IID_IUIAutomationValuePattern, (void **) &valuePattern);
                                                        if (!FAILED(hr)) {
                                                            hr = valuePattern->get_CurrentValue(&url);
                                                            if (FAILED(hr)) {
                                                                qInfo("[GetFirefoxURL] Failed to get url value, HR: 0x%08x\n\n", hr);
                                                                failedBit = true;
                                                            } // else: success
                                                            valuePattern->Release();
                                                        } else {
                                                            qInfo("[GetFirefoxURL] Failed to get value pattern, HR: 0x%08x\n\n", hr);
                                                            failedBit = true;
                                                        }
                                                        patternInter->Release();
                                                    } else {
                                                        qInfo("[GetFirefoxURL] Failed to get value pattern interface, HR: 0x%08x\n\n", hr);
                                                        failedBit = true;
                                                    }
                                                    urlElement->Release();
                                                } else {
                                                    qInfo("[GetFirefoxURL] Failed to get edit of address toolbar, HR: 0x%08x\n\n", hr);
                                                    failedBit = true;
                                                }
                                                editCondition->Release();
                                            } else {
                                                qInfo("[GetFirefoxURL] Failed to get edit condition, HR: 0x%08x\n\n", hr);
                                                failedBit = true;
                                            }
                                            comboElement->Release();
                                        } else {
                                            qInfo("[GetFirefoxURL] Failed to get comboBox of address toolbar, HR: 0x%08x\n\n", hr);
                                            failedBit = true;
                                        }
                                        comboCondition->Release();
                                    } else {
                                        qInfo("[GetFirefoxURL] Failed to get comboBox condition, HR: 0x%08x\n\n", hr);
                                        failedBit = true;
                                    }
                                    toolbarElement->Release();
                                } else {
                                    qInfo("[GetFirefoxURL] Failed to get address toolbar, HR: 0x%08x\n\n", hr);
                                    failedBit = true;
                                }
                            } else {
                                qInfo("[GetFirefoxURL] Too less Firefox's toolbars, %d\n\n", length);
                                failedBit = true;
                            }
                            toolbars->Release();
                        } else {
                            qInfo("[GetFirefoxURL] Failed to get Firefox toolbars length, HR: 0x%08x\n\n", hr);
                            failedBit = true;
                        }
                    } else {
                        qInfo("[GetFirefoxURL] Failed to get Firefox toolbars, HR: 0x%08x\n\n", hr);
                        failedBit = true;
                    }
                    toolbarCondition->Release();
                } else {
                    qInfo("[GetFirefoxURL] Failed to get toolbar condition, HR: 0x%08x\n\n", hr);
                    failedBit = true;
                }
                firefoxElement->Release();
            } else {
                qInfo("[GetFirefoxURL] Failed to ElementFromHandle, HR: 0x%08x\n\n", hr);
                failedBit = true;
            }
            _automation->Release();
        } else {
            qInfo("[GetFirefoxURL] Failed to create a CUIAutomation, HR: 0x%08x\n", hr);
            failedBit = true;
        }
        CoUninitialize();
    } else {
        qInfo("[GetFirefoxURL] CoInitialize failed, HR:0x%08x\n", hr);
        failedBit = true;
    }

    if(failedBit){
        return returnedError;
    } else {
        return std::wstring(url, SysStringLen(url));
    }
}

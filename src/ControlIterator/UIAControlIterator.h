#include "IControlIterator.h"

#include <uiautomation.h>
#include <strsafe.h>
#include <src/DataCollector/WindowEvents_W.h>

class UIAControlItem : public IControlItem
{
public:
    IUIAutomationElement *node;
    UIAControlItem(IControlItem *parent = NULL, IUIAutomationElement *node = NULL)
            : IControlItem(parent), node(node) {};
    std::wstring getDescription();
    std::wstring getRoleText();
    long getRole();
    std::wstring getName();
    std::wstring getValue();

};

class UIAControlIterator : public IConttrolIterator
{
public:
    static bool informError;
    int timeout;

    UIAControlIterator() : IConttrolIterator()
    {
        this->timeout = 500;
    }

    template<class C>
    void iterate(HWND currenthwnd, C *callbackObj, bool (C::*callbackFunction)(IControlItem *node, void *userData),
                 void *userData = NULL, bool fromLast = false)
    {
        if (WindowEvents_W::getWindowsVersion() < 6) {
            return;
        }

        SYSTEMTIME time;
        GetSystemTime(&time);
        this->started = (time.wSecond * 1000) + time.wMilliseconds;

        HRESULT res = CoInitialize(NULL);
        if (!SUCCEEDED(res)) {
            if (!UIAControlIterator::informError) {
                UIAControlIterator::informError = true;
                qDebug("Couldn't initialize COM library %x", res);
            }
            return;
        }

        res = InitializeUIAutomation(&g_pAutomation);
        if (!SUCCEEDED(res)) {
            if (!UIAControlIterator::informError) {
                UIAControlIterator::informError = true;
                qDebug("Couldn't initialize UIA %x", res);
            }

            CoUninitialize();
            return;
        }

        IUIAutomationElement *elem;
        res = g_pAutomation->ElementFromHandle(currenthwnd, &elem);
        if (elem && SUCCEEDED(res)) {
            UIAControlItem root(NULL, elem);
            iterateRecursion(&root, callbackObj, callbackFunction, userData, fromLast);
        }
        CoUninitialize();
    }

private:
    long int started;
    IUIAutomation *g_pAutomation;

    HRESULT InitializeUIAutomation(IUIAutomation **ppAutomation)
    {
        return CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, reinterpret_cast<void **>(ppAutomation));
    }

    template<class C>
    bool iterateRecursion(UIAControlItem *parent, C *callbackObj,
                          bool (C::*callbackFunction)(IControlItem *node, void *userData), void *userData,
                          bool fromLast)
    {
        bool result = true;
        if (parent == NULL) {
            return result;
        }

        SYSTEMTIME time;
        GetSystemTime(&time);
        long int miliseconds = (time.wSecond * 1000) + time.wMilliseconds;
        if (miliseconds - this->started > this->timeout) {
            return false;
        }

        IUIAutomationTreeWalker *pControlWalker = NULL;
        IUIAutomationElement *pNode = NULL;

        g_pAutomation->get_ControlViewWalker(&pControlWalker);
        if (pControlWalker != NULL) {
            pControlWalker->GetFirstChildElement(parent->node, &pNode);
            if (pNode != NULL) {
                while (pNode) {
                    UIAControlItem node(parent, pNode);

                    result = (callbackObj->*callbackFunction)(&node, userData);

                    if (result) {
                        result = iterateRecursion(&node, callbackObj, callbackFunction, userData, fromLast);
                    }

                    IUIAutomationElement *pNext = NULL;

                    if (result) {
                        pControlWalker->GetNextSiblingElement(pNode, &pNext);
                    }
                    pNode->Release();
                    pNode = pNext;
                }
            }
        }

        if (pControlWalker != NULL) {
            pControlWalker->Release();
        }

        if (pNode != NULL) {
            pNode->Release();
        }

        return result;
    }
};
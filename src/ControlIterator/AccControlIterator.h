#include "IControlIterator.h"

#include <Oleacc.h>
#include <strsafe.h>

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <QtCore/qlogging.h>

#pragma comment(lib, "oleacc.lib")

class AccControlItem : public IControlItem
{
public:
    IAccessible *node;
    AccControlItem(IControlItem *parent = NULL, IAccessible *node = NULL) : IControlItem(parent), node(node) {};
    std::wstring getDescription();
    std::wstring getRoleText();
    long getRole();
    std::wstring getName();
    std::wstring getValue();

};

class AccControlIterator : public IConttrolIterator
{
public:

    template<class C>
    void iterate(HWND currenthwnd, C *callbackObj, bool (C::*callbackFunction)(IControlItem *node, void *userData),
                 void *userData = NULL, bool fromLast = false)
    {
        IAccessible *acc_main = 0;
        HRESULT hresult;

        hresult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        if (!SUCCEEDED(hresult)) {
            std::stringstream ss;
            ss << "Error code = 0x" << std::hex << hresult << std::endl;
//            LOOP_LOG("HRESULT:" + ss.str());
            return;
        }
        hresult = AccessibleObjectFromWindow(currenthwnd, NULL, IID_IAccessible, (LPVOID * ) & acc_main);

        if (hresult == S_OK) {
            if (acc_main) {
                AccControlItem *item = new AccControlItem(NULL, acc_main);

                iterateRecursion(item, callbackObj, callbackFunction, userData, fromLast);

                item->node->Release();
                delete item;
            }
        }

        CoUninitialize();
    }

private:

    template<class C>
    bool iterateRecursion(AccControlItem *parent, C *callbackObj,
                          bool (C::*callbackFunction)(IControlItem *node, void *userData), void *userData,
                          bool fromLast)
    {
        long obtained = 0;
        long child_count = 0;
        HRESULT hresult;

        if (parent->getRole() == ROLE_SYSTEM_LINK) {
            return true;
        }
        bool result = true;

        hresult = parent->node->get_accChildCount(&child_count);
        VARIANT *children = new VARIANT[child_count];
        hresult = AccessibleChildren(parent->node, 0, child_count, children, &obtained);

        if (hresult == S_OK) {
            for (int i = fromLast * (obtained - 1); (i < obtained && !fromLast) || (i >= 0 && fromLast); i += 1 - (fromLast * 2)) {
                if (children[i].vt == VT_DISPATCH) {

                    if (children[i].pdispVal) {
                        if (result) {
                            IAccessible *acc_child = NULL;
                            if (children[i].pdispVal->QueryInterface(IID_IAccessible, (LPVOID * ) & acc_child) == S_OK) {
                                AccControlItem node(parent, acc_child);

                                result = (callbackObj->*callbackFunction)(&node, userData);

                                if (result) {
                                    result = iterateRecursion(&node, callbackObj, callbackFunction, userData, fromLast);
                                }

                                if (node.node) {
                                    node.node->Release();
                                }
                            }
                        }
                        children[i].pdispVal->Release();
                    }
                }
            }
        }

        delete[]children;
        return result;
    }
};

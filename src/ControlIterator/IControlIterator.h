#pragma once

#include <string>
#include <Windows.h>

class IControlItem
{
public:
    IControlItem *parent;
    int level;

    IControlItem(IControlItem *parent = NULL) : parent(parent), level(parent ? parent->level + 1 : 0) {};

    virtual std::wstring getDescription() { return L""; };
    virtual std::wstring getRoleText() { return L""; };
    virtual long getRole() { return 0; };
    virtual std::wstring getName() { return L""; };
    virtual std::wstring getValue() { return L""; };
};

class IConttrolIterator
{
public:
    IConttrolIterator(void);
    ~IConttrolIterator(void);

    template<class C>
    void iterate(HWND currenthwnd, C *callbackObj, bool (C::*callbackFunction)(IControlItem *node, void *userData),
                 void *userData = NULL, bool fromLast = false) = 0;
};

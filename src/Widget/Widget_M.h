#ifndef TIMECAMPDESKTOP_WIDGET_M_H
#define TIMECAMPDESKTOP_WIDGET_M_H

#pragma once

#include "Widget.h"

#include <QtMac>

class Widget_M : public Widget
{
public:
    void setText(QString title) override;
    void setMenu(QMenu *menu) override;
    void setIcon(QString iconPath) override;
    void showMe() override;
    void hideMe() override;
    bool isHidden() override;

    Widget_M();
    ~Widget_M();

private:
    void *macWidget;
};


#endif //TIMECAMPDESKTOP_WIDGET_M_H

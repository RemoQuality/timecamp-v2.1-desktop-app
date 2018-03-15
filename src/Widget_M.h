#ifndef TIMECAMPDESKTOP_WIDGET_M_H
#define TIMECAMPDESKTOP_WIDGET_M_H

#pragma once

#include "Widget.h"

class Widget_M : public Widget
{
public:
    void setTaskTitle(QString title) override;
    void showMe() override;
    void hideMe() override;
    bool isHidden() override;

    Widget_M();
    ~Widget_M();

private:
    void * macWidget;
};


#endif //TIMECAMPDESKTOP_WIDGET_M_H

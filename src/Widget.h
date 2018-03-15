#ifndef TIMECAMPDESKTOP_WIDGET_H
#define TIMECAMPDESKTOP_WIDGET_H

#include <QString>

class Widget
{
public:
    virtual void setTaskTitle(QString title) = 0;
    virtual void showMe() = 0;
    virtual void hideMe() = 0;
    virtual bool isHidden() = 0;
};

#endif //TIMECAMPDESKTOP_WIDGET_H

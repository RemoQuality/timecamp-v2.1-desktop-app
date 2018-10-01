#ifndef TIMECAMPDESKTOP_WIDGET_H
#define TIMECAMPDESKTOP_WIDGET_H

#include <QString>
#include <QMenu>

class Widget
{
public:
    virtual void setTimerText(QString) = 0;
    virtual void setTaskText(QString) = 0;
    virtual void setMenu(QMenu *) = 0;
    virtual void setIcon(QString iconPath) = 0;
    virtual void showMe() = 0;
    virtual void hideMe() = 0;
    virtual bool isHidden() = 0;
    virtual ~Widget() = default;
};

#endif //TIMECAMPDESKTOP_WIDGET_H

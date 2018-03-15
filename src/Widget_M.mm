#include "Widget_M.h"
#include "Widget_M_Cocoa.h"

Widget_M::Widget_M()
{
    macWidget = [[Widget_M_Cocoa alloc] init];
}

Widget_M::~Widget_M()
{
    [macWidget release];
}

void Widget_M::setTaskTitle(QString title)
{
    [macWidget SetTaskTitle:title];
}

void Widget_M::showMe()
{
    [macWidget ShowMe];
}

void Widget_M::hideMe()
{
    [macWidget HideMe];
}

bool Widget_M::isHidden()
{
    return [macWidget IsHidden];
}

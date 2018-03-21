#include <objc/NSObject.h>
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


void Widget_M::setIcon(QIcon icon)
{
    QPixmap pixmap = icon.pixmap(16, 16);
    NSImage * nsimage = QtMac::toNSImage(pixmap);
    [macWidget SetImage:nsimage];
}

void Widget_M::setMenu(QMenu* menu)
{
    [macWidget SetMenu:menu];
}

void Widget_M::setText(QString title)
{
    [macWidget SetText:title];
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

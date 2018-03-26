#include <objc/NSObject.h>
#include "Widget_M.h"
#include "Widget_M_Cocoa.h"

#include <QPixmap>
#include <QWindow>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QDebug>
#include <cmath>

Widget_M::Widget_M()
{
    macWidget = [[Widget_M_Cocoa alloc] init];
}

Widget_M::~Widget_M()
{
    [macWidget release];
}


void Widget_M::setIcon(QString iconPath)
{
    QPixmap pixmap = QPixmap(iconPath);
//    pixmap.setMask(pixmap.createHeuristicMask());
    int pixelRatio = std::lround(((QGuiApplication*)QCoreApplication::instance())->devicePixelRatio());
    NSImage * nsimage = QtMac::toNSImage(pixmap.scaledToWidth(16 * pixelRatio, Qt::SmoothTransformation));

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

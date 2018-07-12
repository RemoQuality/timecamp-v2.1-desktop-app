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
    qDebug() << "[Widget_M] created";
}

Widget_M::~Widget_M()
{
    [macWidget release];
    qDebug() << "[Widget_M] destroyed";
}

void Widget_M::setIcon(QString iconPath)
{
    QPixmap pixmap = QPixmap(iconPath);
//    pixmap.setMask(pixmap.createHeuristicMask());
//    qreal commaRatio = ((QGuiApplication *) QCoreApplication::instance())->devicePixelRatio();
//#ifdef Q_OS_MACOS
//    if (commaRatio > 1.0f) {
//        pixmap.setDevicePixelRatio(commaRatio);
//    }
//#endif
    NSImage *nsimage = QtMac::toNSImage(pixmap.scaledToWidth(32, Qt::SmoothTransformation));
//    NSImage *nsimage = QtMac::toNSImage(pixmap);

    [macWidget SetImage:nsimage];
}

void Widget_M::setMenu(QMenu *menu)
{
    [macWidget SetMenu:menu];
}

void Widget_M::setTimerText(QString title)
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

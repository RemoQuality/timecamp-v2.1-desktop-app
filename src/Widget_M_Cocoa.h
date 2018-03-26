#ifndef TIMECAMPDESKTOP_WIDGET_M_COCOA_H
#define TIMECAMPDESKTOP_WIDGET_M_COCOA_H

#pragma once

#include <QString>
#include <QMenu>

#include <AppKit/AppKit.h>
#include <AppKit/NSStatusItem.h>
#include <Foundation/Foundation.h>

#import "Widget_M.h"

@interface Widget_M_Cocoa : NSObject
{
    NSStatusItem* widget;
    NSString *widgetText;

    NSFont* stdFont;
    NSDictionary* attributesStd;
    NSAttributedString *attributedWidgetEmpty;
    NSAttributedString *attributedWidgetText;
    bool isHidden;
}

- (id) init;
- (void) SetText: (QString) text;
- (void) SetMenu: (QMenu*) menu;
- (void) SetImage: (NSImage*) nsimage;
- (void) HideMe;
- (void) ShowMe;
- (bool) IsHidden;

@end

#endif //TIMECAMPDESKTOP_WIDGET_M_COCOA_H

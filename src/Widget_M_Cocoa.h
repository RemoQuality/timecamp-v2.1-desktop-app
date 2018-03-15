#ifndef TIMECAMPDESKTOP_WIDGET_M_COCOA_H
#define TIMECAMPDESKTOP_WIDGET_M_COCOA_H

#pragma once

#include <QString>

#include <AppKit/AppKit.h>
#include <AppKit/NSStatusItem.h>
#include <Foundation/Foundation.h>

#import "Widget_M.h"

@interface Widget_M_Cocoa : NSObject
{
    NSStatusItem* widget;
}

- (id) init;
- (void) SetTaskTitle: (QString) title;
- (void) HideMe;
- (void) ShowMe;
- (bool) IsHidden;

@end

#endif //TIMECAMPDESKTOP_WIDGET_M_COCOA_H

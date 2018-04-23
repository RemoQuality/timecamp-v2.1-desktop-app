#import "Widget_M_Cocoa.h"
#import "src/Settings.h"

@implementation Widget_M_Cocoa

- (id)init {
    self = [super init];

    if (self != nullptr) {
        widget = [[NSStatusBar.systemStatusBar statusItemWithLength:NSVariableStatusItemLength] retain]; //90 to optymalny rozmiar

        NSWidgetText = @"";
        [widget setHighlightMode:YES];
        [widget setEnabled:YES];
//        [widget setTitle:@""];

        stdFont = [NSFont fontWithName:@"Courier" size:16];
        if ([NSFont respondsToSelector:@selector(monospacedDigitSystemFontOfSize:weight:)] != 0) {
            stdFont = [NSFont monospacedDigitSystemFontOfSize:14 weight:NSFontWeightRegular];
        }
        attributesStd = @{NSFontAttributeName: stdFont};

        attributedWidgetEmpty = [NSAttributedString.alloc initWithString:@"" attributes:attributesStd];

        [widget setAttributedTitle:[NSAttributedString.alloc initWithString:NSWidgetText attributes:attributesStd]];

        [attributedWidgetText release];
    }

    return self;
}

- (void)SetText:(QString)text {
    widgetText = text;
    NSWidgetText = widgetText.toNSString();
//    [widget.button setTitle:widgetText];
}

- (void)SetMenu:(QMenu *)menu {
    NSMenu *nsmenu = menu->toNSMenu();
    [widget setMenu:nsmenu];
}

- (void)SetImage:(NSImage *)nsimage {
    [nsimage lockFocus];
    [widget setImage:nsimage];
//    [nsimage setScalesWhenResized:YES];
    [nsimage setSize: NSMakeSize(16, 16)];
    [nsimage setTemplate:YES];
    [nsimage unlockFocus];
//    [widget.button setImage:nsimage];
//    widget.button.image = nsimage;
}

- (void)ShowMe {
    @autoreleasepool {
        NSWidgetText = widgetText.toNSString();

        [widget setAttributedTitle:[NSAttributedString.alloc initWithString:NSWidgetText attributes:attributesStd]];
        isHidden = false;
    }
}

- (void)HideMe {
    @autoreleasepool {
        [widget setAttributedTitle:attributedWidgetEmpty];
        isHidden = true;
    }
}

- (bool)IsHidden {
    return isHidden;
//    return [widget length] == 0; // equals 0, because of NSVariableStatusItemLength (-1)
}

@end

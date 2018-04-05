#import "Widget_M_Cocoa.h"
#import "Settings.h"

@implementation Widget_M_Cocoa

- (id)init {
    self = [super init];

    if (self) {
        widget = [[NSStatusBar.systemStatusBar statusItemWithLength:NSVariableStatusItemLength] retain]; //90 to optymalny rozmiar

        widgetText = @"";
        [widget setHighlightMode:YES];
        [widget setEnabled:YES];
        [widget setTitle:@""];

        stdFont = [NSFont fontWithName:@"Courier" size:16];
        if ([NSFont respondsToSelector:@selector(monospacedDigitSystemFontOfSize:weight:)]) {
            stdFont = [NSFont monospacedDigitSystemFontOfSize:14 weight:NSFontWeightRegular];
        }
        attributesStd = @{NSFontAttributeName: stdFont};

        attributedWidgetText = [NSAttributedString.alloc initWithString:widgetText attributes:attributesStd];

        [widget setAttributedTitle:attributedWidgetText];

        [attributedWidgetText release];
    }

    return self;
}

- (void)SetText:(QString)text {
    widgetText = text.toNSString();
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
    attributedWidgetText = [NSAttributedString.alloc initWithString:widgetText attributes:attributesStd];
    [widget setAttributedTitle:attributedWidgetText];
    [attributedWidgetText release];
    isHidden = false;
//    [widget setTitle:widgetText];
//    [widget setLength:NSVariableStatusItemLength];
}

- (void)HideMe {
    attributedWidgetEmpty = [NSAttributedString.alloc initWithString:@"" attributes:attributesStd];
    [widget setAttributedTitle:attributedWidgetEmpty];
    [attributedWidgetEmpty release];
    isHidden = true;
//    [widget setTitle:@""];
//    [widget setLength:0];
}

- (bool)IsHidden {
    return isHidden;
//    return [widget length] == 0; // equals 0, because of NSVariableStatusItemLength (-1)
}

@end

#import "Widget_M_Cocoa.h"
#import "Settings.h"

@implementation Widget_M_Cocoa

- (id) init
{
    self = [super init];

    if (self)
    {
        widget = [[[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength] retain]; //90 to optymalny rozmiar

        [widget setHighlightMode:YES];
        [widget setEnabled:YES];
        [widget setTitle:@NO_TIMER_TEXT];
        // This goes where you set up the status item
//        [widget setTarget:self];
//        [widget setAction:@selector(TaskClicked:)];
//        [widget setHighlightMode:true];
//        [self SetText:"No Task"];
        //[widget setLength:70]; //show
        //if(!CONFIG->GetBool(CConfig::BOOL_TT_WIDGET))
//        [widget setLength:0]; //hide

        /* Registering here a callback which will be invoked if dark mode setting is changed. */
//        [[NSDistributedNotificationCenter defaultCenter] addObserver:self selector:@selector(darkModeChanged:) name:@"AppleInterfaceThemeChangedNotification" object:nil];
    }

    return self;
}


- (void) SetText: (QString) text
{
    widgetText = text.toNSString();
    [widget.button setTitle:widgetText];

//    NSMutableAttributedString* taskTitleWithFont = nil;
//    NSFont* bold = [NSFont fontWithName:@"Lucida Grande Bold" size:14];
//    NSFont* std = [NSFont fontWithName:@"Lucida Grande" size:14];
//    NSDictionary* attributesBold = [NSDictionary dictionaryWithObjectsAndKeys:bold, NSFontAttributeName, nil];
//    NSDictionary* attributesStd = [NSDictionary dictionaryWithObjectsAndKeys:std, NSFontAttributeName, nil];
//
//
//    if([taskTitle length] > 13) // 13 + with 'M: ' or 'A: '
//    {
//    taskTitle = [taskTitle substringToIndex:13];
//    }
//
//    NSAttributedString* taskTitle2 = [[NSAttributedString alloc] initWithString:taskTitle attributes:attributesStd];
//
//    [taskTitleWithFont appendAttributedString:taskTitle2];
//
//    [widget setAttributedTitle:taskTitle2];
}

- (void) SetMenu: (QMenu*) menu
{
    NSMenu* nsmenu = menu->toNSMenu();
    [widget setMenu:nsmenu];
}

- (void) SetImage: (NSImage*) nsimage
{
    [nsimage setTemplate:YES];
    [widget setImage:nsimage];
//    [widget.button setImage:nsimage];
//    widget.button.image = nsimage;
}

- (void) ShowMe
{
    [widget.button setTitle:widgetText];
//    [widget setLength:NSVariableStatusItemLength];
}

- (void) HideMe
{
    [widget.button setTitle:@""];
//    [widget setLength:0];
}

- (bool) IsHidden
{
//    return [widget length] == 0; // equals 0, because of NSVariableStatusItemLength (-1)
}

@end

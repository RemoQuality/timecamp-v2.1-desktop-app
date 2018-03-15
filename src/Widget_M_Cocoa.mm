#import "Widget_M_Cocoa.h"

@implementation Widget_M_Cocoa

- (id) init
{
    self = [super init];

    if (self)
    {
        widget = [[[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength] retain]; //90 to optymalny rozmiar
        [widget setTitle:@"00:00:00"];
        // This goes where you set up the status item
//        [widget setTarget:self];
//        [widget setAction:@selector(TaskClicked:)];
//        [widget setHighlightMode:true];
//        [self SetTaskTitle:"No Task"];
        //[widget setLength:70]; //show
        //if(!CONFIG->GetBool(CConfig::BOOL_TT_WIDGET))
        [widget setLength:0]; //hide

        /* Registering here a callback which will be invoked if dark mode setting is changed. */
//        [[NSDistributedNotificationCenter defaultCenter] addObserver:self selector:@selector(darkModeChanged:) name:@"AppleInterfaceThemeChangedNotification" object:nil];
    }

    return self;
}


- (void) SetTaskTitle: (QString) title
{
    NSMutableAttributedString* taskTitleWithFont = nil;
    NSFont* bold = [NSFont fontWithName:@"Lucida Grande Bold" size:14];
    NSFont* std = [NSFont fontWithName:@"Lucida Grande" size:14];
    NSDictionary* attributesBold = [NSDictionary dictionaryWithObjectsAndKeys:bold, NSFontAttributeName, nil];
    NSDictionary* attributesStd = [NSDictionary dictionaryWithObjectsAndKeys:std, NSFontAttributeName, nil];

    NSString* taskTitle = title.toNSString();

    if([taskTitle length] > 13) // 13 + with 'M: ' or 'A: '
    {
    taskTitle = [taskTitle substringToIndex:13];
    }

    NSAttributedString* taskTitle2 = [[NSAttributedString alloc] initWithString:taskTitle attributes:attributesStd];

    [taskTitleWithFont appendAttributedString:taskTitle2];

    /* old REMOTECAMP widget
    // Converting seconds to hours and minutes.
    if(TIMETRACKING == NULL || TIMETRACKING->totalTimeElapsedForDay < 0){
        USR_LOG("[Widget_M_Cocoa::SetTaskTitle] Can't set title, TIMETRACKING is NULL or < 0");
        return;
    }
    ldiv_t hours = ldiv(TIMETRACKING->totalTimeElapsedForDay, 3600);
    ldiv_t minutes = ldiv(hours.rem, 60);

    // Converting time data to string.
    wxDateTime timerForDay;
    wxString timerForDayAsString;

    if (hours.quot >= 24 || minutes.quot >= 60)
    {
        timerForDay.Set(0, 0, 0);
        timerForDayAsString = timerForDay.Format("%H:%M");

        USR_LOG("[Widget_M_Cocoa::SetTaskTitle] Invalid totalTimeElapsedForDay received.");
    }
    else
    {
        timerForDay.Set(hours.quot, minutes.quot, 0);
        timerForDayAsString = timerForDay.Format("%H:%M");

        DEBUG_LOG("[Widget_M_Cocoa::SetTaskTitle] Title has been set.");
    }

    DEBUG_LOG(wxString::Format("[Widget_M_Cocoa::SetTaskTitle] Total time elapsed for day: %ld", TIMETRACKING->totalTimeElapsedForDay));
    DEBUG_LOG(wxString::Format("[Widget_M_Cocoa::SetTaskTitle] Hours: %ld", hours.quot));
    DEBUG_LOG(wxString::Format("[Widget_M_Cocoa::SetTaskTitle] Minutes %ld", minutes.quot));

    NSString* taskTitle = [NSString stringWithUTF8String:(const char*)timerForDayAsString.mb_str(wxConvUTF8)];
    NSAttributedString* taskTitle2 = [[NSAttributedString alloc] initWithString:taskTitle attributes:attributesStd];

    taskTitleWithFont = [[NSMutableAttributedString alloc] initWithString:@"" attributes:attributesBold];
    [taskTitleWithFont appendAttributedString:taskTitle2];
    */

    [widget setAttributedTitle:taskTitleWithFont];
}

- (void) ShowMe
{
    [widget setLength:NSVariableStatusItemLength];
}

- (void) HideMe
{
    /*
     Hide widget.

     If you're looking to just hide the NSStatusItem view,
     just call [yourStatusItem setLength:0]
     */
    [widget setLength:0];
}

- (bool) IsHidden
{
    return [widget length] == 0; // equals 0, because of NSVariableStatusItemLength (-1)
}

@end

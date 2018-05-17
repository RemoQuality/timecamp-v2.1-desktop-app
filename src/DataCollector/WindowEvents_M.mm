#include "WindowEvents_M.h"

#include <CoreFoundation/CoreFoundation.h>
#include <Foundation/Foundation.h>

#include <AppKit/AppKit.h>

#include <QTimer>

#include "src/FirefoxUtils.h"

//#import <Foundation/Foundation.h>

// mixed Objective-C and C++ kudos to:
// https://stackoverflow.com/questions/9080619/nsworkspace-notifications-in-cfnotificationcenter

@interface MDWorkspaceWatcher : NSObject
{
    WindowEvents_M *WindowEvents_M;
}
- (id)initWithMyClass:(WindowEvents_M *)aWindowEvents_M;
@end

@implementation MDWorkspaceWatcher
- (id)initWithMyClass:(WindowEvents_M *)aWindowEvents_M {
    if ((self = [super init])) {
        WindowEvents_M = aWindowEvents_M;
        [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self selector:@selector(didActivateApp:) name:NSWorkspaceDidActivateApplicationNotification object:nil];
    }
    return self;
}
// very important:
- (void)dealloc {
    [[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver:self];
    [super dealloc];
}
- (void)didActivateApp:(NSNotification *)notification {
    if (!WindowEvents_M->isIdle) {
        NSRunningApplication *temp = notification.userInfo[NSWorkspaceApplicationKey];
        WindowEvents_M->GetActiveApp(QString::fromNSString(temp.localizedName));
    }
}
@end

unsigned long WindowEvents_M::getIdleTime()
{
    int64_t idlesecs = -1;
    io_iterator_t iter = 0;

    if (IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching("IOHIDSystem"), &iter) == KERN_SUCCESS) {
        io_registry_entry_t entry = IOIteratorNext(iter);
        if (entry) {
            CFMutableDictionaryRef dict = nil;
            if (IORegistryEntryCreateCFProperties(entry, &dict, kCFAllocatorDefault, 0) == KERN_SUCCESS) {
                CFNumberRef obj = (CFNumberRef) CFDictionaryGetValue(dict, CFSTR("HIDIdleTime"));
                if (obj) {
                    int64_t nanoseconds = 0;
                    if (CFNumberGetValue(obj, kCFNumberSInt64Type, &nanoseconds)) {
                        idlesecs = (nanoseconds / 1000000); // Convert from nanoseconds to milliseconds
                    }
                }
                CFRelease(dict);
            }
            IOObjectRelease(entry);
        }
        IOObjectRelease(iter);
    }
    return (unsigned long) idlesecs;
}

void WindowEvents_M::run()
{
    qInfo("thread started");

//    QTimer *timer = new QTimer();
//    connect(timer, SIGNAL(timeout()), this, SLOT(GetActiveApp()));
//    connect(timer, &QTimer::timeout, this, &WindowEvents_M::GetActiveApp);
//    timer->start(2*1000);

//    timer->stop();


//    this->workspaceWatcher = [[MDWorkspaceWatcher alloc] initWithMyClass:this];
    this->initAppleScript();
    while (!QThread::currentThread()->isInterruptionRequested()) {
        // empty loop, waiting for stopping the thread
        if (!isIdle) {
            this->GetActiveApp();
        }
        QThread::msleep(1024); // ~1.5 seconds sleep but pow of 2, almost like in old app
    }

//    [(MDWorkspaceWatcher *) this->workspaceWatcher release];
    [(NSAppleScript *) getWindowScriptObj release];

    qInfo("thread stopped");
}

void WindowEvents_M::initAppleScript()
{
    getWindowScriptObj = [[NSAppleScript alloc] initWithSource:@"global frontApp, frontAppName, windowTitle, fileURL \n \
                                   set windowTitle to \"\" \n \
                                   set fileURL to \"\" \n \
                                   tell application \"System Events\" \n \
                                       set frontApp to first application process whose frontmost is true \n \
                                       set frontAppName to name of frontApp \n \
                                       tell process frontAppName \n \
                                           if exists (1st window whose value of attribute \"AXMain\" is true) then \n \
                                               tell (1st window whose value of attribute \"AXMain\" is true) \n \
                                                   if exists (attribute \"AXDocument\") then \n \
                                                       set fileURL to value of attribute \"AXDocument\" \n \
                                                   end if \n \
                                                   if exists (attribute \"AXTitle\") then \n \
                                                       set windowTitle to value of attribute \"AXTitle\" \n \
                                                   end if \n \
                                               end tell \n \
                                           end if \n \
                                       end tell \n \
                                   end tell \n \
                                   return {frontAppName, windowTitle}"]; //return {frontApp, frontAppName, windowTitle}"];
}

void WindowEvents_M::GetActiveApp(QString processName)
{
    QString appTitle = "";
    QString additionalInfo = "";

    if (processName == "") {
        @autoreleasepool {
            NSString *procName = @"";
            NSArray *runningApps = [[NSWorkspace sharedWorkspace] runningApplications];


            for (id currApp in runningApps) {
                /*
                 Here we suppose that there is only one active app.
                */
//        procName2 = [currApp localizedName];
//        QString buff2 = QString::fromNSString(procName2);
//        qDebug() << "Buffer: " << buff2;
                if ([currApp isActive] != 0) {
                    procName = [currApp localizedName];
                    break;
                }
            }

            QString buffer = QString::fromNSString(procName);
//    qDebug() << "Buffer: " << buffer;

            if (!buffer.isEmpty()) {
                processName = buffer;
            }

            /*
            if(
               (processName == "WinAppHelper" || processName == "fluidapp" || processName == "FluidApp" || (processName.isEmpty() && ([tmpName isEqualToString:@"java"] || [tmpName isEqualToString:@"WinAppHelper"] || [tmpName isEqualToString:@"winapphelper"] || [tmpName isEqualToString:@"(null)"] || [tmpName isEqualToString:@""]))
               || ( [tmpName isEqualToString:@"FluidApp"] || [tmpName isEqualToString:@"fluidapp"] )
               || ( [tmpName isEqualToString:@"prl_client"] || [tmpName isEqualToString:@"prl_client_app"] )
                ))
            {
                processName = GetProcNameFromPath(); //todo: takes a lot of processor this Apple Script
            }
            */
        }
    }

    //Get Window Name or
    appTitle = GetProcWindowName(processName);

    AppData *app;

    app = WindowEvents::logAppName(processName, appTitle, additionalInfo);
    additionalInfo = GetAdditionalInfo(processName.toLower());

    if(additionalInfo != "") {
        app->setAdditionalInfo(additionalInfo); // after we get the URL, update additionalInfo
    }
}

QString WindowEvents_M::GetProcWindowName(QString processName)
{
    QString appTitle("");

    if(getWindowScriptObj == nil){
        return appTitle;
    }
    @autoreleasepool {
        NSString *windowName = @"";
        NSDictionary *errorDict;
        NSAppleEventDescriptor *returnDescriptor = nil;

        // Run the AppleScript.
        returnDescriptor = [(NSAppleScript*) getWindowScriptObj executeAndReturnError:&errorDict];
//    NSLog(@"DESCRIPTOR %@", returnDescriptor);
//    NSLog(@"ERROR %@", errorDict); // warning, THROWS ERRORS! breaks app!!!

        NSInteger howMany = [returnDescriptor numberOfItems];

        if ([returnDescriptor descriptorType] != 0u) {
            if (kAENullEvent != [returnDescriptor descriptorType]) {
                windowName = [[returnDescriptor descriptorAtIndex:howMany] stringValue];
                appTitle = QString::fromNSString(windowName);
            }
        }

    }
    return appTitle;
}

/*
QString WindowEvents_M::GetProcNameFromPath(QString processName)
{
    @autoreleasepool {
        const char *procNameUtf8;
        NSString *procName;
        NSDictionary *errorDict;
        NSAppleEventDescriptor *returnDescriptor = nil;
        // WORKING SCRIPT!
        NSAppleScript *getWindowScriptObj = [[NSAppleScript alloc] initWithSource:@"tell application \"System Events\" \n \
                                   set name_ to name of (info for (path to frontmost application)) as string \n \
                                   end tell \n \
                                   RemoveFromString(name_, \".app\") \n \
                                   on RemoveFromString(theText, CharOrString) \n \
                                   local ASTID, theText, CharOrString, lst \n \
                                   set ASTID to AppleScript's text item delimiters \n \
                                   try \n \
                                   considering case \n \
                                   if theText does not contain CharOrString then ¬ \n \
                                   return theText \n \
                                   set AppleScript's text item delimiters to CharOrString \n \
                                   set lst to theText's text items \n \
                                   end considering \n \
                                   set AppleScript's text item delimiters to ASTID \n \
                                   return lst as text \n \
                                   on error eMsg number eNum \n \
                                   set AppleScript's text item delimiters to ASTID \n \
                                   error \"Can't RemoveFromString: \" & eMsg number eNum \n \
                                   end try \n \
                                   end RemoveFromString"];
        // Run the AppleScript.
        returnDescriptor = [getWindowScriptObj executeAndReturnError:&errorDict];
        //NSLog(@"DESCRIPTOR %@", returnDescriptor);
        [getWindowScriptObj release];

        if ([returnDescriptor descriptorType] != 0u) {
            //NSLog(@"Script executed sucessfully.");
            if (kAENullEvent != [returnDescriptor descriptorType]) {
                //NSLog(@"descriptorType == %@", NSFileTypeForHFSTypeCode(descriptorType));
                procName = [returnDescriptor stringValue];
                procNameUtf8 = [procName UTF8String];
                processName = procNameUtf8;
                //wxMessageBox(processName + "2");
                //DEBUG_LOG("Proc: "+processName);
            }
            //else NSLog(@"AppleScript has no result.");
        }
        //else NSLog(@"Script execution error: %@", [errorDict objectForKey: @"NSAppleScriptErrorMessage"]);
        [returnDescriptor release];
    }
    return processName;
}
*/

QString WindowEvents_M::GetAdditionalInfo(QString processName)
{
    QString additionalInfo("");
    @autoreleasepool {

        bool executed = false;
        const char *addInfoUtf8;
        NSString *addInfo;
        NSDictionary *errorDict;
        NSAppleEventDescriptor *returnDescriptor;
        NSAppleScript *scriptObject = nil;

        // gist for some browsers:
        // https://gist.github.com/vitorgalvao/5392178

        if (processName == "google chrome" || processName == "google chrome canary" || processName == "chromium" || processName == "vivaldi") {
            QString pom2("tell application \"" + processName + "\" \n \
                      get URL of active tab of first window \n \
                      end tell");

            /*
            std::string stdstring_pom2 = pom2.toStdString();
            char * cstring_pom2 = new char [stdstring_pom2.length()+1];
            std::strcpy (cstring_pom2, stdstring_pom2.c_str());

            NSString* pom = [NSString stringWithUTF8String:cstring_pom2];
             */
            NSString *pom = pom2.toNSString();

            /*getWindowScriptObj = [[NSAppleScript alloc] initWithSource:
                            @"tell application \"Google Chrome\" \n \
                            get URL of active tab of first window \n \
                            end tell"];*/
            scriptObject = [[NSAppleScript alloc] initWithSource:pom];
            executed = true;

        } else if (processName == "safari" || processName == "safari technology preview" || processName == "webkit") {
            QString pom2("tell application \"" + processName + "\" \n \
                        get URL of current tab of window 1 \n \
                        end tell");

            NSString *pom = pom2.toNSString();

            scriptObject = [[NSAppleScript alloc] initWithSource:pom];
            executed = true;

        } else if (processName == "opera") {
            scriptObject = [[NSAppleScript alloc] initWithSource:@"tell application \"Opera\" \n \
                        return URL of front document as string \n \
                        end tell"];
            executed = true;

        } else if (processName == "firefox") {
            additionalInfo = getCurrentURLFromFirefox();
            qDebug() << "[FirefoxURL] Found: " << additionalInfo;

            executed = false; //this should be false, because we don't have apple script object initialize here
        }

        if (executed) //jesli skrypt wykonal sie, odczytujemy wynik
        {
            returnDescriptor = [scriptObject executeAndReturnError:&errorDict];
            //NSLog(@"DESCRIPT %@", returnDescriptor);

            if ([returnDescriptor descriptorType] != 0u) {
                // The script execution succeeded.
                //NSLog(@"Script executed sucessfully.");
                executed = true;
            }

            if (kAENullEvent != [returnDescriptor descriptorType]) {
                //NSLog(@"DESCRIPT %@", returnDescriptor);
                //NSLog(@"URL %@", [returnDescriptor stringValue]); //tutaj url
                addInfo = [returnDescriptor stringValue];
                addInfoUtf8 = [addInfo UTF8String];
                additionalInfo = addInfoUtf8;
                //DEBUG_LOG("URL: "+additionalInfo);
            }
            //else NSLog(@"AppleScript has no result.");
        }
        [scriptObject release];
    }
    return additionalInfo;
}

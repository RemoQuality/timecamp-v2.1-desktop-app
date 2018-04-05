#include <MacTypes.h>

#include <Foundation/NSAppleScript.h>
#include <Foundation/NSProcessInfo.h>

#include <AppKit/NSApplication.h>
#include <objc/NSObject.h>

#include <QString>
#include <QMessageBox>
#include "Settings.h"

void enableAssistiveDevices()
{
    /* Enabling assistive devices for OSX. */
    SInt32 OSXversionMajor, OSXversionMinor;
    if (Gestalt(gestaltSystemVersionMajor, &OSXversionMajor) == noErr && Gestalt(gestaltSystemVersionMinor, &OSXversionMinor) == noErr) {
        if (OSXversionMajor > 10 || (OSXversionMajor == 10 && OSXversionMinor >= 9)) {

            // https://stackoverflow.com/questions/17693408/enable-access-for-assistive-devices-programmatically-on-10-9
            NSDictionary *options = @{(id) kAXTrustedCheckOptionPrompt: @NO};
            BOOL accessibilityEnabled = AXIsProcessTrustedWithOptions((CFDictionaryRef) options);

            if (!accessibilityEnabled) {
                QString message(QString() + APPLICATION_NAME + " requires enabled access for Accessibility to work properly. Please enable Accessibility option for " + APPLICATION_NAME + ".");
                QMessageBox::information(nullptr, APPLICATION_NAME, message);

                NSDictionary *errorDict;
                NSAppleEventDescriptor *returnDescriptor;
                NSAppleScript *scriptObject = [[NSAppleScript alloc] initWithSource:@"tell application \"System Preferences\" \n \
                                               activate \n \
                                               set the current pane to pane id \"com.apple.preference.security\" \n \
                                               reveal anchor \"Privacy_Accessibility\" of pane id \"com.apple.preference.security\" \n \
                                               end tell"];

                // Run the AppleScript.
                returnDescriptor = [scriptObject executeAndReturnError:&errorDict];
                [scriptObject release];
            }
        } else {
            if (!AXAPIEnabled()) //ta metoda jest zalezna od wersji systemu - moga wystepowac problemy (choc nie powinny)
            {
                /* Here we call QString() just to be able use "+" operator for more than one const wchar_t* */
                QString message(QString() + APPLICATION_NAME + " requires enabled access for assistive devices to work properly. You will be asked to enter your root password shortly.");

                QMessageBox::information(nullptr, APPLICATION_NAME, message);

                NSDictionary *errorDict;
                NSAppleEventDescriptor *returnDescriptor;
                NSAppleScript *scriptObject = [[NSAppleScript alloc] initWithSource:@"tell application \"System Events\" \n \
                                               set UI elements enabled to true \n \
                                               end tell"];

                // Run the AppleScript.
                returnDescriptor = [scriptObject executeAndReturnError:&errorDict];
                [scriptObject release];

                if ([returnDescriptor descriptorType]) {
                    qDebug("Access for assistive devices enabled");
                } else {
                    qInfo("[CApp::OnInit] Apple script for assistive decives failed");
                }
            }
        }
    }
}
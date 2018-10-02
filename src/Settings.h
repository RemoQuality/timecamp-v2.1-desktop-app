#pragma once
#ifndef THEGUI_SETTINGS_H
#define THEGUI_SETTINGS_H

#define ORGANIZATION_NAME "Time Solutions"
#define ORGANIZATION_DOMAIN "TimeCamp.com"

#define APPLICATION_NAME "TimeCamp Desktop"
#define APPLICATION_VERSION "2.0.0.0"

#define WINDOW_NAME "TimeCamp"
#define MARKETING_URL "https://www.timecamp.com/"
#define LOGIN_URL "https://www.timecamp.com/auth/login"
#define API_URL "https://www.timecamp.com/third_party/api"
#define APPLICATION_URL "https://www.timecamp.com/app#/timesheets/timer"
#define OFFLINE_URL "https://www.timecamp.com/helper/setdate/today/offline"
#define CONTACT_SUPPORT_URL "https://www.timecamp.com/kb/"

#define CONTACT_EMAIL "desktopapp@timecamp.com"

#define NO_TIMER_TEXT "0:00"

#define MAIN_ICON ":/Icons/AppIcon.ico"
#define MAIN_BG ":/Icons/AppIcon.ico"

// db params
#define DB_FILENAME "localdb.sqlite"

// connection params
#define CONN_USER_AGENT "TC Desktop App 2.0"
#define CONN_CUSTOM_HEADER_NAME "X-DAPP"
#define CONN_CUSTOM_HEADER_VALUE "2.0"
#define SETT_API_SERVICE_FIELD "tc_dapp_2_api"

// settings fields
#define SETT_TRACK_PC_ACTIVITIES "TRACK_PC_ACTIVITIES"
#define SETT_TRACK_AUTO_SWITCH "TRACK_AUTO_SWITCH"
#define SETT_SHOW_WIDGET "SHOW_WIDGET"

#define SETT_APIKEY "API_KEY"
#define SETT_LAST_SYNC "LAST_SYNC"
#define SETT_WAS_WINDOW_LEFT_OPENED "WAS_WINDOW_LEFT_OPENED"
#define SETT_IS_FIRST_RUN "IS_FIRST_RUN"

#define SETT_HIDDEN_COMPUTER_ACTIVITIES_CONST_NAME "computer activity"

#define MAX_ACTIVITIES_BATCH_SIZE 400
#define MAX_LOG_TEXT_LENGTH 150

#define KB_SHORTCUTS_START_TIMER "ctrl+alt+shift+."
#define KB_SHORTCUTS_STOP_TIMER "ctrl+alt+shift+,"
#define KB_SHORTCUTS_OPEN_WINDOW "ctrl+alt+shift+/"

#endif //THEGUI_SETTINGS_H

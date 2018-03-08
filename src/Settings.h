#pragma once
#ifndef THEGUI_SETTINGS_H
#define THEGUI_SETTINGS_H

#define ORGANIZATION_NAME "Time Solutions"
#define ORGANIZATION_DOMAIN "TimeCamp.com"

#define APPLICATION_NAME "TimeCamp Desktop"
#define APPLICATION_VERSION "2.0.0.0"

#define WINDOW_NAME "TimeCamp"
#define APPLICATION_URL "https://www.timecamp.com/dashboard"

#ifdef __linux__
#define MAIN_ICON "~/Documents/timecamp-desktop/res/AppIcon.ico"
#elif _WIN32
#define MAIN_ICON "D:\NewApp\TheGUI\res\AppIcon.ico"
#else
#define MAIN_ICON "/Users/timecamp/Documents/timecamp-desktop/res/AppIcon.ico"
#endif

// db params
#define DB_FILENAME "localdb.sqlite"

// connection params
#define CONN_USER_AGENT "TC Desktop App 2.0"
#define CONN_CUSTOM_HEADER_NAME "X-DAPP"
#define CONN_CUSTOM_HEADER_VALUE "2.0"

// settings fields
#define SETT_TRACK_PC_ACTIVITIES "TRACK_PC_ACTIVITIES"
#define SETT_APIKEY "API_KEY"
#define SETT_LAST_SYNC "LAST_SYNC"
#define SETT_WAS_WINDOW_LEFT_OPENED "WAS_WINDOW_LEFT_OPENED"

#endif //THEGUI_SETTINGS_H

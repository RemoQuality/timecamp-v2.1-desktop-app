# Reference: http://qt-project.org/doc/qt-5.0/qtdoc/cmake-manual.html

cmake_minimum_required(VERSION 3.1)
# https://doc.qt.io/qt-5.10/cmake-manual.html
# CMake 3.1.0 adds the right compiler flags for us, require it

# Add folder where are supportive functions
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(QtCommon) # cmake/QtCommon.cmake

include(CMakeProjectConfig.cmake)
project(TimeCampDesktop VERSION ${PRODUCT_VERSION})

if (NOT DEFINED PRODUCT_NAME)
    message(FATAL_ERROR "You haven't prepared CMakeProjectConfig.cmake file.
    See README.md/Developing for details.")
endif ()

# Set PROJECT_VERSION_PATCH and PROJECT_VERSION_TWEAK to 0 if not present, needed by add_project_meta
fix_project_version()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    if ("${CMAKE_GENERATOR}" MATCHES "MinGW")
        message(FATAL_ERROR "There's no QtWebEngine (no chromium) for MinGW, you have to use MSVC on win32")
    elseif ("${CMAKE_GENERATOR}" MATCHES "Visual Studio" OR "${CMAKE_GENERATOR}" MATCHES "NMake" OR "${CMAKE_GENERATOR}" MATCHES "Ninja")
        LIST(APPEND CMAKE_PREFIX_PATH "C:\\WinDDK\\7600.16385.1\\lib\\win7\\i386")
    endif ()
elseif (APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "-framework Carbon -framework AppKit -framework IOKit -framework Foundation -framework CoreFoundation")
endif ()

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5WebEngineWidgets REQUIRED)
find_package(Qt5Sql REQUIRED)

if (UNIX AND NOT APPLE)
    find_package(Qt5X11Extras REQUIRED)
    message("x11 extras loaded")
elseif (APPLE)
    find_package(qt5macextras REQUIRED)
    message("Mac extras loaded")
elseif (WIN32)
    find_package(qt5winextras REQUIRED)
    message("Win extras loaded")
endif ()

foreach (plugin ${Qt5Sql_PLUGINS})
    get_target_property(_loc ${plugin} LOCATION)
    message("Plugin ${plugin} is at location ${_loc}")
    #    if(${plugin} MATCHES "Qt5::QSQLiteDriverPlugin")
    #        list(APPEND TC_LIBS ${_loc})
    #    endif()
endforeach ()

ADD_DEFINITIONS(-DUNICODE -D_UNICODE -DPSAPI_VERSION=1 -DQHOTKEY_LIB -DQHOTKEY_LIB_BUILD -DQT_USE_QSTRINGBUILDER)

if ("${CMAKE_CXX_COMPILER}" MATCHES "clang")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-limit-debug-info")
endif()

list(APPEND SOURCE_FILES
        "src/Settings.h" # a header without cpp file
        "src/main.cpp"
        "src/DbManager.cpp"
        "src/MainWidget.cpp"
        "src/Overrides/TCRequestInterceptor.cpp"
        "src/Overrides/TCNavigationInterceptor.cpp"
        "src/Overrides/TCWebEngineView.cpp"
        "src/Comms.cpp"
        "src/AppData.cpp"
        "src/Task.cpp"
        "src/Autorun.cpp"
        "src/WindowEventsManager.cpp"
        "src/TrayManager.cpp"
        "third-party/mozilla_lz4/lz4.c"
        "third-party/QTLogRotation/logutils.cpp"
        "src/DataCollector/WindowEvents.cpp"
        "src/Widget/Widget.cpp"
        )


if (WIN32)
    list(APPEND SOURCE_FILES
            "src/DataCollector/WindowEvents_W.cpp"
            "src/ControlIterator/IControlIterator.cpp"
            "src/ControlIterator/AccControlIterator.cpp"
            "src/ControlIterator/UIAControlIterator.cpp"
            )

    list(APPEND TC_LIBS
            "oleacc.lib"
            "psapi.lib"
            "Iphlpapi.lib"
            "OleAut32.lib"
            "Ole32.lib"
            "user32.lib"
            "comsuppw.lib"
            )
    set(Qt5_OS_LIBRARIES Qt5::winextras)
endif ()

if (UNIX AND NOT APPLE)
    list(APPEND SOURCE_FILES
            "src/ChromeUtils.cpp"
            "src/FirefoxUtils.cpp"
            "src/DataCollector/WindowEvents_U.cpp"
            )

    list(APPEND TC_LIBS
            "-lX11 -lXss"
            )
    set(Qt5_OS_LIBRARIES Qt5::X11Extras)
endif ()

if (APPLE)
    list(APPEND SOURCE_FILES
            "src/ChromeUtils.cpp"
            "src/FirefoxUtils.cpp"
            "src/DataCollector/WindowEvents_M.mm"
            "src/Utils_M.mm"
            "src/Widget/Widget_M.mm"
            "src/Widget/Widget_M_Cocoa.mm"
            )
    set_source_files_properties(src/*.mm PROPERTIES COMPILE_FLAGS "-x objective-c++")

    set(Qt5_OS_LIBRARIES Qt5::MacExtras)
endif ()

# QHotkey
set(QHOTKEY_PATH "third-party/vendor/de/skycoder42/qhotkey/QHotkey")
list(APPEND SOURCE_FILES
        "${QHOTKEY_PATH}/QHotkey"
        "${QHOTKEY_PATH}/qhotkey.cpp"
        "${QHOTKEY_PATH}/qhotkey_p.h"
        )

if (WIN32)
    list(APPEND SOURCE_FILES
            "${QHOTKEY_PATH}/qhotkey_win.cpp"
            )
elseif (UNIX AND NOT APPLE)
    list(APPEND SOURCE_FILES
            "${QHOTKEY_PATH}/qhotkey_x11.cpp"
            )
elseif (APPLE)
    list(APPEND SOURCE_FILES
            "${QHOTKEY_PATH}/qhotkey_mac.cpp"
            )
endif ()

#if(CMAKE_BUILD_TYPE MATCHES Debug)
#    file(GLOB POCO_LIBS "path/to/debug/libs/*.a")
#elseif(CMAKE_BUILD_TYPE MATCHES Release)
#    file(GLOB POCO_LIBS "path/to/RELEASE/libs/*.a")
#endif()


add_project_meta(META_FILES_TO_INCLUDE)

set(RESOURCE_FILES res/TimeCampDesktop.qrc)


add_executable(${PROJECT_NAME} ${OS_BUNDLE} # Expands to WIN32 or MACOS_BUNDLE depending on OS
        ${SOURCE_FILES} ${META_FILES_TO_INCLUDE} ${RESOURCE_FILES}
        )

if (WIN32)
    # hide windows console...
    if ("${CMAKE_GENERATOR}" MATCHES "MinGW")
        target_compile_options(${PROJECT_NAME} PUBLIC -Wl,--subsystem,windows -mwindows)
    elseif ("${CMAKE_GENERATOR}" MATCHES "Visual Studio" OR "${CMAKE_GENERATOR}" MATCHES "NMake" OR "${CMAKE_GENERATOR}" MATCHES "Ninja")
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
        set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS")
        set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
    endif ()
    #elseif(APPLE)
    #    set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
endif ()

set(Qt5_LIBRARIES Qt5::Core Qt5::Gui Qt5::Network Qt5::Widgets Qt5::WebEngineWidgets Qt5::Sql)
target_link_libraries(${PROJECT_NAME} ${TC_LIBS} ${Qt5_LIBRARIES} ${Qt5_OS_LIBRARIES})

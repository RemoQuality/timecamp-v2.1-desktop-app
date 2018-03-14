# Reference: http://qt-project.org/doc/qt-5.0/qtdoc/cmake-manual.html

cmake_minimum_required(VERSION 3.0)

# Add folder where are supportive functions
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(QtCommon) # cmake/QtCommon.cmake



project(TimecampDesktop VERSION 1.0)

# Set PROJECT_VERSION_PATCH and PROJECT_VERSION_TWEAK to 0 if not present, needed by add_project_meta
fix_project_version()

# Set additional project information
set(COMPANY "Time Solutions")
set(COPYRIGHT "Copyright (c) 2010-2018 Time Solutions. All rights reserved.")
set(IDENTIFIER "com.TimeSolutions.TimecampDesktop")


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    if ("${CMAKE_GENERATOR}" MATCHES "MinGW")
    set(CMAKE_PREFIX_PATH
        "C:\\Qt\\5.10.0\\mingw53_32\\lib\\cmake" # what about mingw64?
        # also, there's no QtWebEngine (no chromium) for MinGW, so we're stuck with MSVC on win32
    )
    elseif("${CMAKE_GENERATOR}" MATCHES "Visual Studio" OR "${CMAKE_GENERATOR}" MATCHES "NMake" OR "${CMAKE_GENERATOR}" MATCHES "Ninja")
    set(CMAKE_PREFIX_PATH
        "C:\\Qt\\5.10.0\\msvc2017_64\\lib\\cmake"
        "C:\\WinDDK\\7600.16385.1\\lib\\win7\\i386"
    )
    endif()
elseif(UNIX AND NOT APPLE)
    set(CMAKE_PREFIX_PATH "~/Qt/5.10.1/gcc_64/")
else()
    set(CMAKE_PREFIX_PATH "~/Qt/5.10.1/clang_64/")
    set(CMAKE_EXE_LINKER_FLAGS "-framework AppKit -framework IOKit -framework Foundation -framework CoreFoundation")
endif()

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5WebEngineWidgets REQUIRED)
find_package(Qt5Sql REQUIRED)

foreach(plugin ${Qt5Sql_PLUGINS})
    get_target_property(_loc ${plugin} LOCATION)
    message("Plugin ${plugin} is at location ${_loc}")
#    if(${plugin} MATCHES "Qt5::QSQLiteDriverPlugin")
#        list(APPEND TC_LIBS ${_loc})
#    endif()
endforeach()

ADD_DEFINITIONS(-DUNICODE -D_UNICODE -DPSAPI_VERSION=1)

list(APPEND SOURCE_FILES
    "src/Settings.h" # a header without cpp file
    "src/main.cpp"
    "src/DbManager.cpp"
    "src/MainWidget.cpp"
    "src/TCRequestInterceptor.cpp"
    "src/TCNavigationInterceptor.cpp"
    "src/TCWebEngineView.cpp"
    "src/Comms.cpp"
    "src/AppData.cpp"
    "src/Autorun.cpp"
    "src/FirefoxUtils.cpp"
    "src/WindowEventsManager.cpp"
    "src/TrayManager.cpp"
    "src/third-party/mozilla_lz4/lz4.c"
    "src/WindowEvents.cpp" # a header without cpp file
)


if(WIN32)
    list(APPEND SOURCE_FILES
        "src/WindowEvents_W.cpp"
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
endif()

if(UNIX AND NOT APPLE)
    list(APPEND SOURCE_FILES
        "src/WindowEvents_U.cpp"
    )

    list(APPEND TC_LIBS
        "-lX11 -lXss"
    )
endif()

if(APPLE)
    list(APPEND SOURCE_FILES
         "src/WindowEvents_M.mm"
        )
    set_source_files_properties(src/WindowEvents_M.mm PROPERTIES COMPILE_FLAGS "-x objective-c++")

#    list(APPEND TC_LIBS
#
#        )
endif()



#if(CMAKE_BUILD_TYPE MATCHES Debug)
#    file(GLOB POCO_LIBS "path/to/debug/libs/*.a")
#elseif(CMAKE_BUILD_TYPE MATCHES Release)
#    file(GLOB POCO_LIBS "path/to/RELEASE/libs/*.a")
#endif()


add_project_meta(META_FILES_TO_INCLUDE)

set(RESOURCE_FILES TimecampDesktop.qrc)


add_executable(${PROJECT_NAME} ${OS_BUNDLE} # Expands to WIN32 or MACOS_BUNDLE depending on OS
    ${SOURCE_FILES} ${META_FILES_TO_INCLUDE} ${RESOURCE_FILES}
)

if(WIN32)
    # hide windows console...
    if ("${CMAKE_GENERATOR}" MATCHES "MinGW")
        target_compile_options(TimecampDesktop PUBLIC -Wl,--subsystem,windows -mwindows)
    elseif("${CMAKE_GENERATOR}" MATCHES "Visual Studio" OR "${CMAKE_GENERATOR}" MATCHES "NMake" OR "${CMAKE_GENERATOR}" MATCHES "Ninja")
        set_target_properties(TimecampDesktop PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
        set_target_properties(TimecampDesktop PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
        set_target_properties(TimecampDesktop PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS")
        set_target_properties(TimecampDesktop PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
        set_target_properties(TimecampDesktop PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
        set_target_properties(TimecampDesktop PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
    endif()
#elseif(APPLE)
#    set_target_properties(TimecampDesktop PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
endif()

target_link_libraries(TimecampDesktop ${TC_LIBS} ${Qt5Core_QTMAIN_LIBRARIES})

qt5_use_modules(TimecampDesktop Core Gui Network Widgets WebEngineWidgets Sql)

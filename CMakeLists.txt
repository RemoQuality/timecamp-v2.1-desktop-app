cmake_minimum_required(VERSION 3.7)
project(TheGUI)
set(CMAKE_CXX_STANDARD 17)

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
else()
	# UNIX
    set(CMAKE_PREFIX_PATH "~/Qt5.2.0/5.2.0/clang_64/")
endif()

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#include_directory(${CMAKE_CURRENT_SOURCE_DIR}) # the same?
# Instruct CMake to run moc automatically when needed.
#qt5_wrap_ui(UI_HEADERS MainWidget.ui)
#qt5_add_resources(UI_RESOURCES bsed.qrc)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

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

list(APPEND TC_SOURCES
	"src/Settings.h" # a header without cpp file
	"src/main.cpp"
    "src/DbManager.cpp"
	"src/MainWidget.cpp"
	"src/TCRequestInterceptor.cpp"
	"src/Comms.cpp"
	"src/AppData.cpp"
    "src/Autorun.cpp"
    "src/FirefoxUtils.cpp"
    "src/WindowEventsManager.cpp"
    "src/TrayManager.cpp"
    "src/third-party/mozilla_lz4/lz4.c"
	"src/WindowEvents.h" # a header without cpp file
)


if(WIN32)
    list(APPEND TC_SOURCES
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
    list(APPEND TC_SOURCES
		"src/WindowEvents_U.cpp"
	)

    list(APPEND TC_LIBS
		"-lX11"
	)
endif()


#if(CMAKE_BUILD_TYPE MATCHES Debug)
#    file(GLOB POCO_LIBS "path/to/debug/libs/*.a")
#elseif(CMAKE_BUILD_TYPE MATCHES Release)
#    file(GLOB POCO_LIBS "path/to/RELEASE/libs/*.a")
#endif()


add_executable(TheGUI ${TC_SOURCES} ${UI_HEADERS})

if(WIN32)
	# hide windows console...
    if ("${CMAKE_GENERATOR}" MATCHES "MinGW")
        target_compile_options(TheGUI PUBLIC -Wl,--subsystem,windows -mwindows)
    elseif("${CMAKE_GENERATOR}" MATCHES "Visual Studio")
        set_target_properties(TheGUI PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
        set_target_properties(TheGUI PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
    endif()
endif()

target_link_libraries(TheGUI ${TC_LIBS} ${Qt5Core_QTMAIN_LIBRARIES})

qt5_use_modules(TheGUI Core Gui Network Widgets WebEngineWidgets Sql)

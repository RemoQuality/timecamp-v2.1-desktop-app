# Timecamp Desktop

Manage your tasks and timesheet on Timecamp, right from your desktop.
Start timers, create projects, do everything that can be done on the web version.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See *Creating installers* for notes on how to make packages for Windows, macOS and Linux.

### Prerequisites

To compile **Timecamp Desktop** you need:
* _[Qt](https://www.qt.io/) 5.9_ or greater, and


* either a CMake-compatible IDE (Qt Creator, JetBrains CLion, Microsoft Visual Studio 2017) - they can load this repo as a native project
* ... or just [CMake](https://cmake.org/) _(at least v. 3.1)_, which can generate project files for many other IDEs and build systems
(eg. XCode, Code::Blocks, Eclipse, Ninja Build System, older MS Visual Studios, or plain old Makefiles)

## Developing

Start by [Installing Qt](http://doc.qt.io/qt-5/gettingstarted.html).

Make sure you have all of the required tools - install with your favourite package manager:
```
git, cmake, clang/gcc/msvc
```

Get this repo:
```
git clone git@github.com:timecamp/timecamp-desktop.git
```

*(Optional)* Generate project files for your IDE, eg. XCode on Mac:
```
cmake -G Xcode -B cmake-build-xcode
```
Where `Xcode` is [generator of your choosing](https://cmake.org/cmake/help/v3.1/manual/cmake-generators.7.html), and `cmake-build-xcode` is where project files will be created.

Now you can open it in your IDE of choice.

Firstly, you need to make `CMakeProjectConfig.cmake`.
Copy and rename `CMakeProjectConfig.cmake.example` and modify the path to Path to Qt precompiled libs.


## Compiling our source

We compile **Timecamp Desktop** with MSVC on Windows, Clang on macOS and GCC on Linux.
Both Qt Creator and CLion allow you to choose the compiler in settings in their GUI.

Other combinations are sometimes possible, but are not used by us right now.
We already checked these:
* Windows
    * MSVC - Microsoft Visual Studio Compiler
        * you can use it in eg. Qt Creator, CLion and MS Visual Studio
    * not <del>MinGW</del>
        * QtWebEngine [can't be built on Windows](https://doc.qt.io/qt-5.10/qtwebengine-platform-notes.html) using this toolchain, and we depend on QtWebEngine
    * not <del>Clang</del>
        * same as above, but [Chromium for Windows is now being built using Clang by default](https://groups.google.com/a/chromium.org/forum/#!topic/chromium-dev/Y3OEIKkdlu0),
        and QtWebEngine is based on Chromium, so [maybe it will be available soon](https://bugreports.qt.io/browse/QTBUG-66664?)    
* macOS
    * Clang
        * you can use it in eg. Qt Creator, CLion and XCode-generated project
    * not <del>GCC</del>
        * we had some troubles with our macOS-related Objective-C++ mixed code (status widget), with Clang it works out of the box - but you can submit patches to get us gcc compatibility!
* Linux
    * GCC
    * Clang
        * Qt precompiled with gcc is available for download on qt.io,
        but you can use it with clang as well, as it is ABI compatible;
        or you can compile Qt with clang yourself


## Creating Installers

Our installers are created with:
* Windows
    * `windeployqt` - a Qt [Deployment Tool](http://doc.qt.io/qt-5/windows-deployment.html), which pulls all needed Qt dynamic libraries and dependencies
    * `NSIS` - [Nullsoft Scriptable Install System](http://nsis.sourceforge.net/Main_Page), with our proprietary script (not in this repo)
* macOS
    * `macdeployqt` - a Qt [Deployment Tool](http://doc.qt.io/qt-5/osx-deployment.html), which pulls all needed Qt dynamic libraries and dependencies and creates an App Bundle
    * `create-dmg` - a Node.js [script for creating DMG packages](https://github.com/sindresorhus/create-dmg), _created by [Sindre Sorhus](https://github.com/sindresorhus)_
* Linux
    * `linuxdeployqt` - a Community-created [Deployment Tool](https://github.com/probonopd/linuxdeployqt), which pulls all needed Qt dynamic libraries and dependencies
    * `appimagetool` - a [creator of runnable packages](https://github.com/AppImage/AppImageKit) for Linux in AppImage format, _created by [Simon Peter](https://github.com/probonopd) and contributors_

Additionally, our Windows and macOS installers are being signed with Timecamp certificates.

For more details see `deploy_win.bat`, `deploy_mac.sh` and `deploy_lin.sh` scripts.

## Acknowledgments

Source:
* [Qt](https://www1.qt.io/) [LGPLv3] - we're using it under LGPLv3 license, with prebuilt binaries provided on their website
* [QHotkey](https://github.com/Skycoder42/QHotkey) [BSD 3-Clause] - globally working hotkeys for Windows, macOS and Linux
* [LZ4](http://lz4.github.io/lz4/) [BSD 2-Clause] - for reading Firefox data (see `FirefoxUtils.cpp`)
* [Minimal CMake template for Qt5 Projects](https://github.com/euler0/mini-cmake-qt) [Unlicense] - the simplest possible version of a working, multi-platform Qt5 project
* Previous [Timecamp Desktop App](https://github.com/timecamp/timecamp-v2-desktop-app) - sources for macOS widgets and data collecting

Tools:
* [EditorConfig](http://editorconfig.org/) - consistent coding style (see `.editorconfig` for more info)

## Authors

* **[Karol Olszacki](https://github.com/karololszak)** - *Initial work*

See also the list of [contributors](https://github.com/timecamp/timecamp-desktop/contributors) who participated in this project.


#!/bin/sh
# add Qt bin to your $PATH

SRC="cmake-build-relwithdebinfo-gcc"
TARGET="linuxdeployqt/TimecampDesktop.AppDir"
CMAKE_NAME="TimecampDesktop"
BUNDLE_NAME="TimecampDesktop"
LinuxDeployQtExec="/home/karololszak/Downloads/linuxdeployqt-continuous-x86_64.AppImage"
AppImageToolExec="/home/karololszak/Downloads/appimagetool-x86_64.AppImage"

# as in https://github.com/probonopd/linuxdeployqt#simplest-example
rm -rf $TARGET
mkdir -p $TARGET/usr/bin
mkdir -p $TARGET/usr/lib
mkdir -p $TARGET/usr/share/applications # app.desktop file path
mkdir -p $TARGET/usr/share/icons/hicolor/48x48 # icons

rm -rf $SRC
mkdir -p $SRC
cd $SRC
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_MAKE_PROGRAM=/usr/bin/make -DCMAKE_C_COMPILER=/usr/bin/gcc -DCMAKE_CXX_COMPILER=/usr/bin/g++ -G "Unix Makefiles" "../"
cd ../
cmake --build $SRC --target "${CMAKE_NAME}" -- # build
cp $SRC/"${CMAKE_NAME}" $TARGET"/usr/bin/""${BUNDLE_NAME}" # copy the binary
cp res"/${BUNDLE_NAME}.desktop" $TARGET"/usr/share/applications/${BUNDLE_NAME}.desktop" # copy app.desktop file
cp "res/AppIcon128.png" $TARGET"/usr/share/icons/hicolor/48x48/${BUNDLE_NAME}.png" # copy app.desktop file

chmod a+x "${LinuxDeployQtExec}"
${LinuxDeployQtExec} $TARGET"/usr/share/applications/${BUNDLE_NAME}.desktop" -bundle-non-qt-libs -verbose=2
cp /lib/x86_64-linux-gnu/libssl.so.1.0.0  $TARGET"/usr/lib/"
cp /lib/x86_64-linux-gnu/libcrypto.so.1.0.0  $TARGET"/usr/lib/"
${LinuxDeployQtExec} $TARGET"/usr/share/applications/${BUNDLE_NAME}.desktop" -appimage -verbose=2
#${AppImageToolExec} $TARGET

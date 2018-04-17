#!/bin/sh
# add Qt bin to your $PATH

SRC="cmake-build-relwithdebinfo-gcc-48"
TARGET="linuxdeployqt/TimeCampDesktop.AppDir"
CMAKE_NAME="TimeCampDesktop"
BUNDLE_NAME="TimeCampDesktop"
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

#source /opt/qt*/bin/qt*-env.sh ## call it from shell yourself

unset QTDIR; unset QT_PLUGIN_PATH ; unset LD_LIBRARY_PATH

chmod a+x "${LinuxDeployQtExec}"
cp /lib/x86_64-linux-gnu/libssl.so.1.0.0  $TARGET"/usr/lib/"
cp /lib/x86_64-linux-gnu/libcrypto.so.1.0.0  $TARGET"/usr/lib/"
cp /lib/x86_64-linux-gnu/libbsd.so.0 $TARGET"/usr/lib/"
#cp /lib/x86_64-linux-gnu/libexpat.so.1 $TARGET"/usr/lib/"
#cp /lib/x86_64-linux-gnu/libz.so.1 $TARGET"/usr/lib/"
#cp /lib/x86_64-linux-gnu/libglib-2.0.so.0 $TARGET"/usr/lib/"
#cp /lib/x86_64-linux-gnu/libpcre.so.3 $TARGET"/usr/lib/"
${LinuxDeployQtExec} $TARGET"/usr/share/applications/${BUNDLE_NAME}.desktop" -bundle-non-qt-libs -extra-plugins=sqldrivers,iconengines -verbose=2
${LinuxDeployQtExec} $TARGET"/usr/share/applications/${BUNDLE_NAME}.desktop" -appimage -verbose=2
#${AppImageToolExec} $TARGET

echo "Unbundled dependant libs (they need to be in the system!):"
find $TARGET -executable -type f -exec ldd {} \; | grep -v "timecamp-desktop" | cut -d " " -f 2-3  | sort | uniq

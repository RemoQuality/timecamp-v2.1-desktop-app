#!/bin/bash
# add Qt bin to your $PATH

SRC="cmake-build-relwithdebinfo-gcc"
TARGET="linuxdeployqt/TimecampDesktop.AppDir"
CMAKE_NAME="TimecampDesktop"
BUNDLE_NAME="TimecampDesktop"

ARCH="amd64"
VERSION="1.0.1"
TARGET_DEB="linuxdeployqt/deb"

function deb_control_template(){
cat <<-EOF
Package: Timecamp-Desktop
Version: $VERSION
Section: contrib/comm
Priority: optional
Installed-Size: 8096
Architecture: $ARCH
Depends: libc-bin, libdbus-1-3, libgtk2.0-0, liblzma5, libsqlcipher0, libsqlite3-0
Maintainer: Time Solutions <desktop@timecamp.com>
Description: Client application for TimeCamp software.
 The package contains client application for www.timecamp.com software.
EOF
}

rm -rf $TARGET_DEB

mkdir -p $TARGET_DEB/DEBIAN
deb_control_template > $TARGET_DEB/DEBIAN/control

mkdir -p $TARGET_DEB/usr/bin
mkdir -p $TARGET_DEB/usr/share/applications
mkdir -p $TARGET_DEB/usr/share/icons/hicolor/48x48
mkdir -p $TARGET_DEB/usr/share/"${BUNDLE_NAME}"


cp -f $SRC/"${CMAKE_NAME}" $TARGET_DEB"/usr/share/${BUNDLE_NAME}/${BUNDLE_NAME}" # copy the binary
cp "res/AppIcon128.png" $TARGET_DEB"/usr/share/${BUNDLE_NAME}/${BUNDLE_NAME}.png" # copy icon
cp res"/${BUNDLE_NAME}.desktop" $TARGET_DEB"/usr/share/${BUNDLE_NAME}/${BUNDLE_NAME}.desktop" # copy app.desktop file

# fix .desktop file paths
sed -i.bak "s/Name=TimecampDesktop/Name=Timecamp Desktop/g" $TARGET_DEB"/usr/share/${BUNDLE_NAME}/${BUNDLE_NAME}.desktop"
sed -i.bak "s/Icon=TimecampDesktop/Icon=\/usr\/share\/${BUNDLE_NAME}\/${BUNDLE_NAME}.png/g" $TARGET_DEB"/usr/share/${BUNDLE_NAME}/${BUNDLE_NAME}.desktop"
sed -i.bak "s/Icon=TimecampDesktop/Icon=\/usr\/share\/${BUNDLE_NAME}\/${BUNDLE_NAME}.png/g" $TARGET_DEB"/usr/share/${BUNDLE_NAME}/${BUNDLE_NAME}.desktop"

rm $TARGET_DEB"/usr/share/${BUNDLE_NAME}/${BUNDLE_NAME}.desktop.bak" # copy app.desktop file

# link files

ln -s "../share/${BUNDLE_NAME}/${BUNDLE_NAME}" $TARGET_DEB"/usr/bin/""${BUNDLE_NAME}"
ln -s "../../share/${BUNDLE_NAME}/${BUNDLE_NAME}.desktop" $TARGET_DEB"/usr/share/applications/${BUNDLE_NAME}.desktop"
ln -s "../../../../share/${BUNDLE_NAME}/${BUNDLE_NAME}.png" $TARGET_DEB"/usr/share/icons/hicolor/48x48/${BUNDLE_NAME}.png"

find $TARGET_DEB -type d -exec chmod 0755 {} \;
find $TARGET_DEB -type f -exec chmod 0644 {} \;
chmod a+x $TARGET_DEB/usr/bin/TimecampDesktop

fakeroot dpkg --build $TARGET_DEB ./

TCFILE='timecamp-desktop_'$VERSION'_'$ARCH;

tar -czvf $TCFILE.tar.gz ./$TCFILE.deb ./install_deb.sh

#!/bin/sh
# add Qt bin to your $PATH, and change path to the cmake-generated.app file

# copied from StackOverflow and improved: https://stackoverflow.com/questions/27952111/unable-to-sign-app-bundle-using-qt-frameworks-on-os-x-10-10
BUNDLE_NAME="TimeCamp Desktop.app"
DMG_NAME="TimeCamp-Desktop-2.0.dmg"
TARGET="cmake-build-relwithdebinfo"

rm -rf ${TARGET:?}/"${BUNDLE_NAME}"
rm -rf ${TARGET:?}/${DMG_NAME} # dmg HAS TO BE DELETED

mkdir -p $TARGET
cmake --build $TARGET --target TimeCampDesktop --
mv $TARGET/"TimeCampDesktop.app" $TARGET/"TimeCamp Desktop.app"

cd $TARGET

#setup the pasted code

QT_FRAMEWORK_PATH=~/Qt/5.10.1/clang_64/lib
QT_BIN_PATH=~/Qt/5.10.1/clang_64/bin
CERTIFICATE="Developer ID Application"
FRAMEWORKS="QtCore QtGui QtNetwork QtWidgets QtWebEngineWidgets QtSql QtWebEngineCore QtDBus QtPositioning QtPrintSupport QtQml QtQuick QtQuickWidgets QtSvg QtWebChannel"
#BAD_FRAMEWORKS="QtDBus QtPositioning QtPrintSupport QtQml QtQuick QtQuickWidgets"
#BAD_FRAMEWORKS="QtDBus QtPositioning QtPrintSupport QtQml QtQuick QtQuickWidgets QtSvg QtWebChannel QtWebEngineCore QtWebEngineProcess"
BAD_FRAMEWORKS=$FRAMEWORKS

# Run QT tool to deploy
${QT_BIN_PATH}/macdeployqt "${BUNDLE_NAME}"

# FIX ISSUE 6
# Please note that Qt5 frameworks have incorrect layout after SDK build, so this isn't just a problem with `macdeployqt` but whole framework assembly part.
# Present
#   QtCore.framework/
#       Contents/
#           Info.plist
#       QtCore    -> Versions/Current/QtCore
#       Versions/
#           Current -> 5
#           5/
#               QtCore
# After macdeployqt
#   QtCore.framework/
#       Resources/
#       Versions/
#           5/
#               QtCore
#
# Expected
#   QtCore.framework/
#       QtCore    -> Versions/Current/QtCore
#       Resources -> Versions/Current/Resources
#       Versions/
#           Current -> 5
#           5/
#               QtCore
#               Resources/
#                   Info.plist
# So in order to comply with expected layout: https://developer.apple.com/library/mac/documentation/MacOSX/Conceptual/BPFrameworks/Concepts/FrameworkAnatomy.html

for CURRENT_FRAMEWORK in ${FRAMEWORKS}; do
    echo "Processing framework: ${CURRENT_FRAMEWORK}"

    echo "Deleting existing resource folder"
    rmdir "${BUNDLE_NAME}"/Contents/Frameworks/${CURRENT_FRAMEWORK}.framework/Resources

    echo "create resource folder"
    mkdir -p "${BUNDLE_NAME}"/Contents/Frameworks/${CURRENT_FRAMEWORK}.framework/Versions/5/Resources

    echo "create copy resource file"
    cp ${QT_FRAMEWORK_PATH}/${CURRENT_FRAMEWORK}.framework/Contents/Info.plist "${BUNDLE_NAME}"/Contents/Frameworks/${CURRENT_FRAMEWORK}.framework/Versions/5/Resources/

    echo "create symbolic links"
    ln -nfs 5                                     "${BUNDLE_NAME}"/Contents/Frameworks/${CURRENT_FRAMEWORK}.framework/Versions/Current
    ln -nfs Versions/Current/${CURRENT_FRAMEWORK} "${BUNDLE_NAME}"/Contents/Frameworks/${CURRENT_FRAMEWORK}.framework/${CURRENT_FRAMEWORK}
    ln -nfs Versions/Current/Resources            "${BUNDLE_NAME}"/Contents/Frameworks/${CURRENT_FRAMEWORK}.framework/Resources
done

# FIX ISSUE 7
echo "***** Correct Frameworks Info.plist file*****"

for CURRENT_FRAMEWORK in ${BAD_FRAMEWORKS}; do
    echo "Correcting bad framework Info.plist: ${CURRENT_FRAMEWORK}"
    TMP=$(sed 's/_debug//g' "${BUNDLE_NAME}"/Contents/Frameworks/${CURRENT_FRAMEWORK}.framework/Resources/Info.plist)
    echo "$TMP" > "${BUNDLE_NAME}"/Contents/Frameworks/${CURRENT_FRAMEWORK}.framework/Resources/Info.plist
done

# SIGNING FIXED FRAMEWORK
CODESIGN_OPTIONS="--verbose=4"

#echo "******* Sign QtWebEngineProcess ***********"
##codesign --force --verify ${CODESIGN_OPTIONS} --sign "$CERTIFICATE" "${BUNDLE_NAME}"/Contents/Frameworks/QtWebEngineCore.framework/Versions/Current/Helpers/QtWebEngineProcess.app
codesign --force --verify ${CODESIGN_OPTIONS} --sign "$CERTIFICATE" "${BUNDLE_NAME}"/Contents/Frameworks/*.framework/Versions/*/*/*.app

echo "******* Signing Frameworks ***********"
#codesign --force --verify ${CODESIGN_OPTIONS} --sign "$CERTIFICATE" "${BUNDLE_NAME}"/Contents/Frameworks/*.framework
for CURRENT_FRAMEWORK in ${FRAMEWORKS}; do
#    echo "Signing framework: ${CURRENT_FRAMEWORK}"
    codesign --force --verify --deep ${CODESIGN_OPTIONS} --sign "$CERTIFICATE" "${BUNDLE_NAME}"/Contents/Frameworks/${CURRENT_FRAMEWORK}.framework
done

# Sign plugins
echo "******* Signing Plugins ***********"
codesign --force --verify --deep ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" "${BUNDLE_NAME}"/Contents/Plugins/*/*.dylib

# ... Do the same for all plugins

# Sign bundle itself
echo "******* Signing Bundle ***********"
codesign --force --verify --deep ${CODESIGN_OPTIONS} --sign "$CERTIFICATE" "${BUNDLE_NAME}"

# Verify

echo "******* Verify Bundle ***********"
codesign --verify --deep ${CODESIGN_OPTIONS} "${BUNDLE_NAME}"


echo "******* Verify Bundle using spctl ***********"
spctl -a -vvvv "${BUNDLE_NAME}"

echo "******* Create DMG ***********"
create-dmg "${BUNDLE_NAME}"

echo "******* Verify DMG ***********"
codesign --verify --deep ${CODESIGN_OPTIONS} $DMG_NAME


echo "******* Verify DMG using spctl ***********"
spctl  -a -t open --context context:primary-signature -vvvv $DMG_NAME

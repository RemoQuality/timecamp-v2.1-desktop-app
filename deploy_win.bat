REM  best to run from "x64 native tools command prompt"

del /s /q deploy
rmdir /s /q deploy
mkdir deploy
mkdir deploy\resources
copy "cmake-build-release-visual-studio\TimecampDesktop.exe" "deploy"
copy "src\resources\programicon.ico" "deploy\resources\programicon.ico"
REM copy "src\localdb.sqlite" "deploy\localdb.sqlite"
C:\Qt\5.10.0\msvc2017_64\bin\windeployqt.exe deploy

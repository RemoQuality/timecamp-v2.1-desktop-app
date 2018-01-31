REM  best to run from "x64 native tools command prompt"

del /s /q deploy
rmdir /s /q deploy
mkdir deploy
mkdir deploy\resources
copy "cmake-build-release-visual-studio\TheGUI.exe" "deploy"
copy "src\resources\programicon.ico" "deploy\resources\programicon.ico"
REM copy "src\resources\localdb.db" "deploy\resources\localdb.db"
C:\Qt\5.10.0\msvc2017_64\bin\windeployqt.exe deploy

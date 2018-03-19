#include <QDebug>

#include "Autorun.h"
#include "Settings.h"

void Autorun::enableAutorun() {
#ifdef _WIN32
    QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    settings.setValue(APPLICATION_NAME, QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
    settings.sync();
#elif __linux__
    //
#else // macOS
    QString plistFile = ""
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
            "<plist version=\"1.0\">\n"
            "    <dict>\n"
            "        <key>RunAtLoad</key>\n"
            "        <true/>\n"
            "        <key>Label</key>\n"
            "        <string>Timecamp Desktop</string>\n"
            "        <key>ProgramArguments</key>\n"
            "        <array>\n"
            "            <string>/Applications/TimecampDesktop.app/Contents/MacOS/TimecampDesktop</string>\n"
            "        </array>\n"
            "    </dict>\n"
            "</plist>";

    QString filename = QDir::homePath() + "/Library/LaunchAgents/TimecampDesktop.autorun.plist";
    if (!QFile::exists(filename)) {
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << plistFile << endl;
        }
    }
#endif
}

void Autorun::disableAutorun() {
#ifdef _WIN32
    QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    settings.remove(APPLICATION_NAME);
    settings.sync();
#elif __linux__
    //
#else // macOS
    QString filename = QDir::homePath() + "/Library/LaunchAgents/TimecampDesktop.autorun.plist";
    if (QFile::exists(filename)) {
        QFile::remove(filename);
    }
#endif
}

bool Autorun::checkAutorun() {
#ifdef _WIN32
    QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    settings.sync();
    return settings.contains(APPLICATION_NAME);
#elif __linux__
    return false;
#else // macOS
    QString filename = QDir::homePath() + "/Library/LaunchAgents/TimecampDesktop.autorun.plist";
   return QFile::exists(filename);
#endif
}

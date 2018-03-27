#include <QDebug>

#include "Autorun.h"
#include "Settings.h"

void Autorun::enableAutorun() {
#ifdef Q_OS_LINUX
    //
#elif defined(Q_OS_WIN)
    QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    settings.setValue(APPLICATION_NAME, QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
    settings.sync();
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
            "            <string>/Applications/Timecamp Desktop.app/Contents/MacOS/TimecampDesktop</string>\n"
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
#ifdef Q_OS_LINUX
    //
#elif defined(Q_OS_WIN)
    QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    settings.remove(APPLICATION_NAME);
    settings.sync();
#else // macOS
    QString filename = QDir::homePath() + "/Library/LaunchAgents/TimecampDesktop.autorun.plist";
    if (QFile::exists(filename)) {
        QFile::remove(filename);
    }
#endif
}

bool Autorun::checkAutorun() {
#ifdef Q_OS_LINUX
    return false;
#elif defined(Q_OS_WIN)
    QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    settings.sync();
    return settings.contains(APPLICATION_NAME);
#else // macOS
    QString filename = QDir::homePath() + "/Library/LaunchAgents/TimecampDesktop.autorun.plist";
   return QFile::exists(filename);
#endif
}

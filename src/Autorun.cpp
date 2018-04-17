#include <QDebug>

#include "Autorun.h"
#include "Settings.h"

void Autorun::enableAutorun()
{
#ifdef Q_OS_LINUX
    //
    QString desktopFile = ""
            "[Desktop Entry]\n"
            "Version=1.0\n"
            "Comment=\"TimeCamp Desktop 1.0\"\n"
            "Type=Application\n"
            "Name=TimeCamp Desktop\n"
            "Exec=\"/usr/share/Time Solutions/TimeCamp Desktop\"\n"
            "Icon=\"/usr/share/Time Solutions/icon.png\"\n"
            "StartupNotify=false\n"
            "Terminal=false\n"
            "Categories=Office;ProjectManagement;Monitor;Network;\n"
    ;

    QString filename = QDir::homePath() + "/.config/autostart/TimeCamp Desktop.desktop";
    if (!QFile::exists(filename)) {
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << desktopFile << endl;
        }
    }

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
                        "        <string>TimeCamp Desktop</string>\n"
                        "        <key>ProgramArguments</key>\n"
                        "        <array>\n"
                        "            <string>/Applications/TimeCamp Desktop.app/Contents/MacOS/TimeCampDesktop</string>\n"
                        "        </array>\n"
                        "    </dict>\n"
                        "</plist>";

    QString filename = QDir::homePath() + "/Library/LaunchAgents/TimeCampDesktop.autorun.plist";
    if (!QFile::exists(filename)) {
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << plistFile << endl;
        }
    }
#endif
}

void Autorun::disableAutorun()
{
#ifdef Q_OS_LINUX
    //
    QString filename = QDir::homePath() + "/.config/autostart/TimeCamp Desktop.desktop";
    if (QFile::exists(filename)) {
        QFile::remove(filename);
    }
#elif defined(Q_OS_WIN)
    QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    settings.remove(APPLICATION_NAME);
    settings.sync();
#else // macOS
    QString filename = QDir::homePath() + "/Library/LaunchAgents/TimeCampDesktop.autorun.plist";
    if (QFile::exists(filename)) {
        QFile::remove(filename);
    }
#endif
}

bool Autorun::checkAutorun()
{
#ifdef Q_OS_LINUX
    QString filename = QDir::homePath() + "/.config/autostart/TimeCamp Desktop.desktop";
    return QFile::exists(filename);
#elif defined(Q_OS_WIN)
    QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    settings.sync();
    return settings.contains(APPLICATION_NAME);
#else // macOS
    QString filename = QDir::homePath() + "/Library/LaunchAgents/TimeCampDesktop.autorun.plist";
    return QFile::exists(filename);
#endif
}

#include <QDebug>

#include "Autorun.h"
#include "Settings.h"

void Autorun::enableAutorun() {
#ifdef _WIN32
    QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    settings.setValue(APPLICATION_NAME, QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
    settings.sync();
#endif
}

void Autorun::disableAutorun() {
#ifdef _WIN32
    QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    settings.remove(APPLICATION_NAME);
    settings.sync();
#endif
}

bool Autorun::checkAutorun() {
#ifdef _WIN32
    QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    settings.sync();
    return settings.contains(APPLICATION_NAME);
#endif
}

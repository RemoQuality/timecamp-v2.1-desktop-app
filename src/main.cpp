#include "Settings.h"
#include "MainWidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    // Caches are saved in %localappdata%/org_name/app_name
    // Eg. C:\Users\timecamp\AppData\Local\Time Solutions\TimeCamp tracking tool
    // Settings are saved in registry: HKEY_CURRENT_USER\Software\Time Solutions\TimeCamp tracking tool

    QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
    QCoreApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);
    QCoreApplication::setApplicationName(APPLICATION_NAME);
    QCoreApplication::setApplicationVersion(APPLICATION_VERSION);

    // Enable high dpi support
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);
    MainWidget w;

    w.setWindowTitle(WINDOW_NAME);

    return app.exec();
}

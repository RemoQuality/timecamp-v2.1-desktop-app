#ifndef THEGUI_AUTORUN_H
#define THEGUI_AUTORUN_H

#include <QSettings>
#include <QDir>
#include <QtCore/qcoreapplication.h>

class Autorun {
public:
    static void enableAutorun();
    static void disableAutorun();
};


#endif //THEGUI_AUTORUN_H

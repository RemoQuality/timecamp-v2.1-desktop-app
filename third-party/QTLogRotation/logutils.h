#ifndef LOGUTILS_H
#define LOGUTILS_H

// courtesy of https://andydunkel.net/2017/11/08/qt_log_file_rotation_with_qdebug/
// slightly modified

#define LOGSIZE 1024 * 1024 * 5 //log size in bytes - 5MB
#define LOGFILES 10

#include <QObject>
#include <QString>
#include <QDebug>
#include <QDate>
#include <QTime>
#include <QStandardPaths>

namespace LOGUTILS {
    bool initLogging();

    void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

}

#endif // LOGUTILS_H

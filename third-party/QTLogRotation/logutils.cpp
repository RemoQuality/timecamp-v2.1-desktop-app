#include "logutils.h"

#include <ctime>
#include <iomanip>
#include <iostream>

#include <QTime>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QDir>
#include <QFileInfoList>

namespace LOGUTILS {
    static QString logFileName;
    static QString logFolderName;
    QString lastMessage;
    qint64 sameMessageCount = 0;

    void initLogFileName() {
        logFileName = QString(logFolderName + "/Log_%1__%2.txt")
            .arg(QDate::currentDate().toString("yyyy_MM_dd"))
            .arg(QTime::currentTime().toString("hh_mm_ss_zzz"));
    }

    /**
     * @brief deletes old log files, only the last ones are kept
     */
    void deleteOldLogs() {
        QDir dir;
        dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
        dir.setSorting(QDir::Time | QDir::Reversed);
        dir.setPath(logFolderName);

        QFileInfoList list = dir.entryInfoList();
        if (list.size() <= LOGFILES) {
            return; //no files to delete
        } else {
            for (int i = 0; i < (list.size() - LOGFILES); i++) {
                QString path = list.at(i).absoluteFilePath();
                QFile file(path);
                file.remove();
            }
        }
    }

    bool initLogging() {
        // Create folder for logfiles if not exists
        logFolderName = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation).first() + "/logs";

        if (!QDir(logFolderName).exists()) {
            QDir().mkdir(logFolderName);
        }

        std::cout << "[LOG PATH] " << logFolderName.toStdString() << std::endl;

        deleteOldLogs(); //delete old log files
        initLogFileName(); //create the logfile name

        QFile outFile(logFileName);
        if (outFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
            qInstallMessageHandler(LOGUTILS::myMessageHandler);
            return true;
        } else {
            return false;
        }
    }

    void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
        //check file size and if needed create new log!
        {
            QFile outFileCheck(logFileName);
            qint64 size = outFileCheck.size();

            if (size > LOGSIZE) //check current log size
            {
                deleteOldLogs();
                initLogFileName();
            }
        }


        const std::time_t stdtime = std::time(nullptr);
//    std::cout << "UTC:       " << std::put_time(std::gmtime(&stdtime), "%H:%M:%S") << '\n';
//    std::cout << "local:     " << std::put_time(std::localtime(&stdtime), "%H:%M:%S") << '\n';
        char timestring[100];

#ifdef Q_OS_WIN
        struct tm buf;
      gmtime_s(&buf, &stdtime);
      std::strftime(timestring, sizeof(timestring), "%H:%M:%S", &buf);
#else
        std::strftime(timestring, sizeof(timestring), "%H:%M:%S", std::gmtime(&stdtime)); // UTC, localtime for local
#endif

        QString txt;
        txt += "[";
        txt += timestring;
        txt += "] ";
        switch (type) {
            case QtDebugMsg:
                txt += QString("Debug:\t%1").arg(msg);
                break;
            case QtInfoMsg:
                txt += QString("Info:\t%1").arg(msg);
                break;
            case QtWarningMsg:
                txt += QString("Warning:\t%1").arg(msg);
                break;
            case QtCriticalMsg:
                txt += QString("Critical:\t%1").arg(msg);
                break;
            case QtFatalMsg:
                txt += QString("Fatal:\t%1").arg(msg);
                break;
        }

        QFile outFile(logFileName);
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream ts(&outFile);
        if(msg != lastMessage || sameMessageCount > 20) {
            if(sameMessageCount > 0){
                QString repeated;
                repeated.append("^ repeated x").append(QString::number(sameMessageCount));
                ts << repeated << endl;
                std::cout << repeated.toStdString() << std::endl;
            }
            ts << txt << endl;
            std::cout << txt.toStdString() << std::endl;
            sameMessageCount = 0;
        } else {
            sameMessageCount++;
        }
        lastMessage = msg;
    }
}

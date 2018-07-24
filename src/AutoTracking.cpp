#include "AutoTracking.h"
#include <QtCore/QDateTime>
#include <QtCore/QSettings>
#include "DbManager.h"
#include "Settings.h"

AutoTracking &AutoTracking::instance() {
    static AutoTracking _instance;
    return _instance;
}

void AutoTracking::checkAppKeywords(AppData *app) {

    QSettings settings;
    bool autoTracking = settings.value(SETT_TRACK_AUTO_SWITCH, false).toBool();
    if(autoTracking) {
        Task *matchedTask = this->matchActivityToTaskKeywords(app);
        if (matchedTask != nullptr) {
            emit foundTask(matchedTask, false);
        }
    }
}

Task *AutoTracking::matchActivityToTaskKeywords(AppData *app) {
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now > lastUpdate + taskUpdateThreshold) { // if we're past X minutes since last task update

        // insert AppData into a List
        QStringList dataItems;
        dataItems.push_back(app->getAppName());
        dataItems.push_back(app->getWindowName());
        dataItems.push_back(app->getAdditionalInfo());

        for (Task *task: DbManager::instance().getTaskList()) { // in every task
            QStringList taskKeywords = task->getKeywordsList(); // get the KW
            if (!taskKeywords.isEmpty()) { // and if we have KW
                for (const QString &dataWithPotentialKeyword: dataItems) { // in every appdata
                    for (const QString &keyword: taskKeywords) { // check each keyword
                        if (dataWithPotentialKeyword.contains(keyword,
                                                              Qt::CaseInsensitive)) { // and if data contains keyword
                            lastUpdate = now;
                            return task; // return task
                        }
                    }
                }
            }
        }
    }
    return nullptr;
}

AutoTracking::AutoTracking(QObject *parent) : QObject(parent) {

}

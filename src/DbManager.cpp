#include "DbManager.h"
#include "Settings.h"

#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QStandardPaths>

DbManager &DbManager::instance()
{
    static DbManager _instance;
    return _instance;
}

DbManager::DbManager(QObject *parent) : QObject(parent)
{
    qDebug() << "[DB] Starting DB manager!";
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbLocation = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation).first() + "/" + DB_FILENAME;
    qDebug() << "[DB] Location: " << dbLocation;
    m_db.setDatabaseName(dbLocation);

    if (!m_db.open()) {
        qWarning() << "[DB] ERROR0: connection with database fail";
        qWarning() << __FUNCTION__ << m_db.lastError().text();
    } else {
        qDebug() << "[DB] Database: connection ok";
        createTable();
        prepareQueries();
    }
}

DbManager::~DbManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DbManager::isOpen() const
{
    return m_db.isOpen();
}

void DbManager::prepareQueries()
{
    // sorry for making a mess and separating prepared queries from data
    // but this will hopefully prevent crashes
    addAppQuery = QSqlQuery(m_db);
    getAppsQuery = QSqlQuery(m_db);
    addAppQuery.prepare("INSERT INTO apps (ID, app_name, window_name, additional_info, start_time, end_time) VALUES (NULL, ?, ?, ?, ?, ?)");
    getAppsQuery.prepare("SELECT app_name, window_name, additional_info, start_time, end_time FROM apps WHERE start_time > :lastSync LIMIT :maxCount");
}

bool DbManager::createTable()
{
    bool TableCreated = true;

    QSqlQuery createTableQuery;
    createTableQuery.prepare("CREATE TABLE \"apps\" ( `ID` INTEGER PRIMARY KEY AUTOINCREMENT, `app_name` TEXT, `window_name` TEXT, `additional_info` TEXT, `start_time` INTEGER NOT NULL, `end_time` INTEGER NOT NULL )");

    if (!createTableQuery.exec()) {
        qDebug() << "[DB] Warning: Couldn't create the table: one might already exist.";
        TableCreated = false;
    }

    return TableCreated;
}

bool DbManager::saveAppToDb(AppData *app)
{
    bool success = false;
    if (!this->isOpen()) {
        qInfo("[DB] ERROR1 can't query yet - DB is not opened");
        return success;
    }

    const QString &appName = app->getAppName();
    const QString &windowName = app->getWindowName();
    const QString &additionalInfo = app->getAdditionalInfo();
    qint64 start = app->getStart();
    qint64 end = app->getEnd();

    if (start > 0 && end > 0 && !appName.isEmpty()) {
        addAppQuery.addBindValue(appName);
        addAppQuery.addBindValue(windowName);
        addAppQuery.addBindValue(additionalInfo);
        addAppQuery.addBindValue(start);
        addAppQuery.addBindValue(end);

        if (addAppQuery.exec()) {
//            qDebug() << "[DB] app added successfully";
            success = true;
        } else {
            qInfo() << "[DB] ERROR3 adding failed: " << addAppQuery.lastError();
        }
    } else {
        qInfo() << "[DB] ERROR4 adding failed: missing values!";
    }

    return success;
}

QVector<AppData> DbManager::getAppsSinceLastSync(qint64 last_sync)
{
    QVector<AppData> appList;
    if (!this->isOpen()) {
        qInfo("[DB] ERROR2 can't query yet - DB is not opened");
        return appList; // return empty appList if DB is not opened
    }

    getAppsQuery.bindValue(":lastSync", last_sync);
    getAppsQuery.bindValue(":maxCount", MAX_ACTIVITIES_BATCH_SIZE);

    if (getAppsQuery.exec()) {
        int qSize = getAppsQuery.size(); // get count of activities

        if(qSize != -1){
            appList.reserve(qSize + 1); // reserve memory space for the count
        }

        while (getAppsQuery.next()) {
            AppData tempApp;
            tempApp.setAppName(getAppsQuery.value("app_name").toString());
            tempApp.setWindowName(getAppsQuery.value("window_name").toString());
            tempApp.setAdditionalInfo(getAppsQuery.value("additional_info").toString());
            tempApp.setStart(getAppsQuery.value("start_time").toLongLong());
            tempApp.setEnd(getAppsQuery.value("end_time").toLongLong());
            appList.push_back(tempApp);
        }
        appList.squeeze(); // finally remove empty elements (because reserve is just a hint)
    }
    return appList;
}

const QVector<Task> &DbManager::getTaskList() const {
    return taskList;
}

void DbManager::setTaskList(const QVector<Task> &taskList) {
    this->taskList = taskList;
}

void DbManager::addToTaskList(const Task &impTask) {
    taskList.push_back(impTask);
}

void DbManager::clearTaskList() {
    taskList.clear();
}

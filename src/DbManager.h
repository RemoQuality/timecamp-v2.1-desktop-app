#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QVector>
#include <QSqlQuery>

#include "AppData.h"
#include "Task.h"

class DbManager : public QObject {
Q_OBJECT
    Q_DISABLE_COPY(DbManager)

public:
    static DbManager &instance();
    ~DbManager() override;

    bool isOpen() const;

    void prepareQueries();

    /**
     * @brief Creates a new table if it doesn't already exist
     * @return true - 'people' table created successfully, false - table not created
     */
    bool createTable();

    QVector<AppData> getAppsSinceLastSync(qint64 last_sync);

    QVector<Task> taskList; // maybe QSet or QMap later?

    const QVector<Task> &getTaskList() const;
    void setTaskList(const QVector<Task> &taskList);
    void addToTaskList(const Task &);
    void clearTaskList();

public slots:

    /**
     * @brief Add data to db
     * @return true - person added successfully, false - person not added
     */
    bool saveAppToDb(AppData *app);

protected:
    explicit DbManager(QObject *parent = nullptr);

    QSqlDatabase m_db;
    QSqlQuery addAppQuery;
    QSqlQuery getAppsQuery;
};

#endif // DBMANAGER_H

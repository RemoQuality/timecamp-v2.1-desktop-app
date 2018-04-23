#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QVector>
#include <QSqlQuery>

#include "AppData.h"

class DbManager : QObject
{
Q_OBJECT
    Q_DISABLE_COPY(DbManager)

public:
    static DbManager &instance();
    explicit DbManager(QObject *parent = nullptr);
    ~DbManager() override;

    bool isOpen() const;

    void prepareQueries();

    /**
     * @brief Creates a new table if it doesn't already exist
     * @return true - 'people' table created successfully, false - table not created
     */
    bool createTable();

    /**
     * @brief Add data to db
     * @return true - person added successfully, false - person not added
     */
    bool saveAppToDb(AppData *app);

    QVector<AppData *> getAppsSinceLastSync(qint64 last_sync);

protected:
    QSqlDatabase m_db;
    QSqlQuery addAppQuery;
    QSqlQuery getAppsQuery;
};

#endif // DBMANAGER_H

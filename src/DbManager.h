#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include "AppData.h"

class DbManager : QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DbManager)

public:
    static DbManager &instance();
    explicit DbManager(QObject *parent = nullptr);
    ~DbManager();

    bool isOpen() const;

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

    QList getAppsSinceLastSync(qint64 last_sync);

private:
    QSqlDatabase m_db;
};

#endif // DBMANAGER_H

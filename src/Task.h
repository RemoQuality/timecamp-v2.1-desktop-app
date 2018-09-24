#ifndef TIMECAMPDESKTOP_TASK_H
#define TIMECAMPDESKTOP_TASK_H

#include <QString>
#include <QtCore/QVector>

class Task {
private:
    qint64 tc_id;
    QString name;
    QString keywords;
    QStringList keywordsList;

public:
    Task();
    explicit Task(qint64);

    qint64 getTaskId() const;
    void setTaskId(qint64 tc_id);

    const QString &getName() const;
    void setName(const QString &name);

    const QString &getKeywords() const;
    void setKeywords(QString keywords);

    QStringList getKeywordsList();
    void setKeywordsList(QStringList keywordsList);
};


#endif //TIMECAMPDESKTOP_TASK_H

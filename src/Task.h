#ifndef TIMECAMPDESKTOP_TASK_H
#define TIMECAMPDESKTOP_TASK_H

#include <QString>
#include <QtCore/QVector>

class Task {
private:
    int tc_id;
    QString name;
    QString keywords;
    QStringList keywordsList;

public:
    Task();
    Task(int);

    int getTc_id() const;
    void setTc_id(int tc_id);

    const QString &getName() const;
    void setName(const QString &name);

    const QString &getKeywords() const;
    void setKeywords(const QString &keywords);

    const QStringList &getKeywordsList() const;
    void setKeywordsList(const QStringList &keywordsList);
};


#endif //TIMECAMPDESKTOP_TASK_H

#include "Task.h"

#include <QUrl>
#include <utility>

Task::Task()
= default;

Task::Task(qint64 tid) {
    tc_id = tid;
}

qint64 Task::getTaskId() const {
    return tc_id;
}

void Task::setTaskId(qint64 tc_id) {
    this->tc_id = tc_id;
}

const QString &Task::getName() const {
    return name;
}

void Task::setName(const QString &name) {
    this->name = name;
}

const QString &Task::getKeywords() const {
    return keywords;
}

void Task::setKeywords(QString keywords) {
    this->keywords = keywords;
    QStringList keywordsList = keywords.split(',', QString::SkipEmptyParts);
    this->setKeywordsList(keywordsList);
}

QStringList Task::getKeywordsList() {
    return keywordsList;
}

void Task::setKeywordsList(QStringList keywordsList) {
    this->keywordsList = std::move(keywordsList);
}

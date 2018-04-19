#pragma once

#include <QString>

QString getChromeSessionFilePath();
QString getChromeTabsFilePath();
QString getCurrentURLFromChromeConfig(QByteArray &data, QString &windowTitle);

QByteArray readChromeFile(const QString &filename);
QString getCurrentURLFromChrome(QString windowTitle);



//
//  FirefoxUtils.hpp
//  TimeCamp
//
//  Created by Aleksey Dvoryanskiy on 06/20/16
//  Modified by Georgiy Jenkovszky on 05/11/17
//  Ported to Qt by Karol Olszacki on 06/02/18
//
//

#pragma once

#include <QString>

QString getFirefoxConfigFilePath();
QString getCurrentURLFromFirefoxConfig(QString &jsonConfig);

void *readFileToMemory(const char *filename, size_t *readSize);
QString parseJsRecoveryFilePath(const QString &recoveryFilePath);
QString parseJsonlz4RecoveryFilePath(const QString &recoveryFilePath);

bool comparatorGreater(const std::pair<QString, time_t> &left, const std::pair<QString, time_t> &right);
QString getCurrentURLFromFirefox();

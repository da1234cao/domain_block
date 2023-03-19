#pragma once

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace utils {

inline void copyFile(const QString &srcPath, const QString &dstPath) {
  QFile srcFile(srcPath);
  QFile dstFile(dstPath);

  // 创建目标目录
  QFileInfo dstFileInfo(dstPath);
  QDir().mkpath(dstFileInfo.path());

  if (srcFile.exists() && srcFile.open(QIODevice::ReadOnly) &&
      dstFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    dstFile.write(srcFile.readAll());
  }
}

} // namespace utils

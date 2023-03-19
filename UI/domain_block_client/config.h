#pragma once
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QString>

class config {
public:
  static config &instance() {
    static config inst;
    return inst;
  }

  QString get_str(QString section, QString key);
  int get_int(QString section, QString key);

  QString get_conf_path() const { return m_default_path; }

private:
  config();

private:
  QString m_default_path;
  QSettings *m_config = nullptr;
};

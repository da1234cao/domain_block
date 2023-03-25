#include "config.h"
#include "utils.hpp"
#include <QDebug>
#include <QSettings>
#include <QStandardPaths>

config::config() {

  // da1234cao组织下的域名阻断配置文件
  // 暂时没去查这种设置为啥不对
  //  QSettings settings("da1234cao", "domain_block");
  QCoreApplication::setOrganizationName("da1234cao");
  QCoreApplication::setApplicationName("domain_block_client");

  // 获取配置文件路径
  QString appName = QCoreApplication::applicationName();
  QString orgName = QCoreApplication::organizationName();
  m_default_path =
      QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) +
      "/" + orgName + "/" + appName + ".ini";

  // qDebug() << "Copied config file from" << m_default_path;

  // 如果默认路径的配置文件不存在，则拷贝一份
  QFileInfo defaultFile(m_default_path);
  if (!defaultFile.exists()) {
    QString currentPath =
        QCoreApplication::applicationDirPath() + "/" + appName + ".ini";
    qDebug() << currentPath;
    utils::copyFile(currentPath, m_default_path);
  }

  // 加载配置文件
  m_config = new QSettings(m_default_path, QSettings::IniFormat);
}

QString config::get_str(QString section, QString key) {
  QString retval;
  QString section_key = section + "/" + key;
  if (m_config->contains(section_key)) {
    retval = m_config->value(section_key).toString();
  }
  return retval;
}

int config::get_int(QString section, QString key) {
  int retval = 0;
  QString section_key = section + "/" + key;
  if (m_config->contains(section_key)) {
    retval = m_config->value(section_key).toInt();
  }
  return retval;
}

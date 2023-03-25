#include "rules.h"
#include "config.h"
#include <QComboBox>
#include <QDebug>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QPushButton>
#include <QString>
#include <QTcpSocket>
#include <QTextStream>
#include <QtNetwork/QHostInfo>

void rules::set_default_info_path() {
  // 在配置目录下创建一个文件保存信息
  QString conf_path = config::instance().get_conf_path();
  QFileInfo fileInfo(conf_path);
  QString dirPath = fileInfo.dir().absolutePath();
  m_rules_file_path = dirPath + "/rules_table.txt";
}

void rules::add_rule(QString doamin, QString action_str) {
  // 填充内容
  m_rules_table->insertRow(0);
  m_rules_table->setItem(0, 0, new QTableWidgetItem(doamin));
  QComboBox *action = new QComboBox(m_rules_table);
  action->addItem("block");
  action->addItem("allow");
  if (action_str == "block")
    action->setCurrentIndex(0);
  else
    action->setCurrentIndex(1);
  m_rules_table->setCellWidget(0, 1, action);
}

void rules::set_rules_table(QTableWidget *_rules_table) {
  m_rules_table = _rules_table;
  set_default_info_path();
  load_info_from_file();
}

void rules::add() {
  m_rules_table->insertRow(0);
  m_rules_table->setItem(0, 0, new QTableWidgetItem());
  QComboBox *action = new QComboBox(m_rules_table);
  action->addItem("block");
  action->addItem("allow");
  action->setCurrentIndex(0);
  m_rules_table->setCellWidget(0, 1, action);
}

QString rules::get_rules_str() {
  QJsonObject jsonObject;
  jsonObject["name"] = "dacao";
  jsonObject["description"] = "Block the following networks";
  QJsonArray rulesArray;

  // 这部分代码可以用get_table_info()函数进行优化
  int row_count = m_rules_table->rowCount();
  for (int i = 0; i < row_count; i++) {
    // 表格的第0列为域名
    QTableWidgetItem *domain_item = m_rules_table->item(i, 0);
    QString domain = domain_item->text();
    if (domain.isEmpty())
      continue;

    // 表格的第1列为动作
    QString action = "block";
    QWidget *action_item = m_rules_table->cellWidget(i, 1);
    if (QComboBox *comboBox = qobject_cast<QComboBox *>(action_item)) {
      action = comboBox->currentText();
    }

    QHostInfo info = QHostInfo::fromName(domain); // 域名转换成ip
    QJsonArray ipArray;
    if (info.error() != QHostInfo::NoError) {
      //      qDebug() << "Lookup failed:" << info.errorString();
    } else {
      QList<QHostAddress> addresses = info.addresses();
      foreach (QHostAddress address, addresses) {
        // qDebug() << "IP Address:" << address.toString();
        ipArray.append(address.toString());
      }
    }

    QJsonObject ruleObject;
    ruleObject["action"] = action;
    ruleObject["domain"] = domain;
    ruleObject["ip"] = ipArray;

    rulesArray.append(ruleObject);
  }
  jsonObject["rules"] = rulesArray;

  QJsonDocument jsonDoc(jsonObject);
  QString jsonString = QString(jsonDoc.toJson(QJsonDocument::Compact));
  qDebug() << jsonString;
  return jsonString;
}

void rules::send_rules() {
  // 发送这些规则到服务

  // 获取表格中的规则
  QString rulesJsonString = get_rules_str();

  // 创建QTcpSocket对象
  QTcpSocket *socket = new QTcpSocket(this);
  // 连接到服务器
  QString serverIp = config::instance().get_str("server", "ip"); // 服务器IP地址
  int serverPort = config::instance().get_int("server", "port"); // 服务器端口号
  qDebug() << serverIp << ":" << serverPort;
  socket->connectToHost(QHostAddress(serverIp), serverPort);

  // 当服务端接收到完整的json后，服务端会自动断开连接;当连接断开时自动销毁QTcpSocket对象
  connect(socket, &QTcpSocket::disconnected, &QTcpSocket::deleteLater);

  connect(socket, &QTcpSocket::readyRead, [=]() {
    QByteArray data = socket->readAll();
    qDebug() << "Received data: " << data;
  });

  // 发送数据
  socket->write(rulesJsonString.toUtf8());
}

void rules::get_table_info() {
  m_rules_info.clear();

  int row_count = m_rules_table->rowCount();
  for (int i = 0; i < row_count; i++) {
    // 表格的第0列为域名
    QTableWidgetItem *domain_item = m_rules_table->item(i, 0);
    QString domain = domain_item->text();
    if (domain.isEmpty())
      continue;
    // 表格的第1列为动作
    QString action = "block";
    QWidget *action_item = m_rules_table->cellWidget(i, 1);
    if (QComboBox *comboBox = qobject_cast<QComboBox *>(action_item)) {
      action = comboBox->currentText();
    }

    m_rules_info.push_back({domain, action});
  }
}

void rules::save_info_to_file() {
  // 获取表格信息
  get_table_info();

  QFile rules_file(m_rules_file_path);
  if (rules_file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    QTextStream stream(&rules_file);
    for (auto &info : m_rules_info) {
      // 获取tuple的最后一个元素有点麻烦，这里直接使用数字
#ifdef _WIN32
      stream << std::get<0>(info) << m_separator << std::get<1>(info)
             << Qt::endl;
#else
      stream << std::get<0>(info) << m_separator << std::get<1>(info) << endl;
#endif
    }
    rules_file.close();
  }
}

void rules::load_info_from_file() {
  // 初始化表格大小:1x2
  m_rules_table->setRowCount(1);
  m_rules_table->setColumnCount(2);

  // 设置水平头
  m_rules_table->setHorizontalHeaderLabels(QStringList() << "域名"
                                                         << "动作");

  if (!QFile::exists(m_rules_file_path)) {
    // 之前没有保存过信息/第一次使用，插入一个examle
    m_rules_table->setItem(0, 0, new QTableWidgetItem("www.example.com"));
    QComboBox *action = new QComboBox(m_rules_table);
    action->addItem("block");
    action->addItem("allow");
    action->setCurrentIndex(0);
    m_rules_table->setCellWidget(0, 1, action);
    return;
  }

  {
    // 第一行默认是被逐渐向下压的，修正下选项
    m_rules_table->setItem(0, 0, new QTableWidgetItem());
    QComboBox *action = new QComboBox(m_rules_table);
    action->addItem("block");
    action->addItem("allow");
    action->setCurrentIndex(0);
    m_rules_table->setCellWidget(0, 1, action);
  }

  // 读取文件中的rule信息
  QFile file(m_rules_file_path);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream stream(&file);
    while (!stream.atEnd()) {
      QString line = stream.readLine();
      QStringList list = line.split(m_separator);
      m_rules_info.push_back({list[0], list[1]});
    }
    file.close();
  }

  // 根据信息，给表格添加行
  for (int i = 0; i < m_rules_info.size(); i++) {
    QString doamin = std::get<0>(m_rules_info[i]);
    QString action_str = std::get<1>(m_rules_info[i]);
    add_rule(doamin, action_str);
  }
}

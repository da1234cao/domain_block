#pragma once
#include <QTableWidget>
#include <tuple>
#include <vector>

class rules : public QObject {
  Q_OBJECT
public:
  rules() = default;
  void set_rules_table(QTableWidget *_rules_table);
  QString get_rules_str();
  void save_info_to_file();
  void load_info_from_file();
  ~rules() { save_info_to_file(); } // 表格信息记录到文件
public slots:
  void add();        // 表格添加一行
  void send_rules(); // 给服务端发送规则

private:
  // 添加一条默认规则，用于示例
  void add_rule(QString doamin, QString action_str);
  void get_table_info();

private:
  QTableWidget *m_rules_table;
  std::vector<std::tuple<QString, QString>> m_rules_info; // 内存中的规则表
  QString m_rules_file_path; // 硬盘上规则表的存储位置
  QString m_separator = ":"; // 分隔符
  void set_default_info_path();
};

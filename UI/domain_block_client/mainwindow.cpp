#include "mainwindow.h"
#include "./ui_mainwindow.h"
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
#include <QtNetwork/QHostInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  m_rules.set_rules_table(ui->rules_table);

  // 点击按钮，添加一行
  connect(ui->add_button, &QPushButton::clicked, &m_rules, &rules::add);

  // 给服务发送规则
  connect(ui->apply_button, &QPushButton::clicked, &m_rules,
          &rules::send_rules);
}

MainWindow::~MainWindow() { delete ui; }

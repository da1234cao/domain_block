#include "config.h"
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  config::instance();

  MainWindow w;
  w.show();
  return a.exec();
}

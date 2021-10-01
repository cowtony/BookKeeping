#include <QApplication>
#include <QDir>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  QDir::setCurrent(a.applicationDirPath());
  MainWindow mainWindow;
  mainWindow.showMaximized();

  return a.exec();
}

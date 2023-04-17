#include <QApplication>
#include <QDir>
#include "main_window/main_window.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QDir::setCurrent(a.applicationDirPath());
    MainWindow main_window;
    main_window.showMaximized();

    return a.exec();
}

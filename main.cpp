#include <QApplication>
#include <QDir>
#include "home_window/home_window.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QDir::setCurrent(a.applicationDirPath());
    HomeWindow main_window;
    main_window.showMaximized();

    return a.exec();
}

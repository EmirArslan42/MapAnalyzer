#include <QApplication>
#include "mainwindow.h"
#include <QIcon>
#include <QMainWindow>
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    //QIcon appIcon(":/images/TEI_Logo.png");
    QIcon icon;
    icon.addFile(":/images/TEI_Logo.ico", QSize(256, 256));
    //w.setWindowIcon(appIcon);
    w.setWindowIcon(icon);
    w.show();
    return app.exec();
}

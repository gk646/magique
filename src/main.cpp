#include <QApplication>

#include "application/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);


    MainWindow mainWindow = MainWindow();
    mainWindow.show();

    return QApplication::exec();
}
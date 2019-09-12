#include "mainwindow.h"

#include <QApplication>


int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    QCursor cursor(Qt::BlankCursor);
    QApplication::setOverrideCursor(cursor);
    QApplication::changeOverrideCursor(cursor);

    MainWindow w;
    w.show();
    return a.exec();
}

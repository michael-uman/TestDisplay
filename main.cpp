#include "mainwindow.h"
#include "runguard.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    RunGuard guard( "wunderbar.com" );
    if ( !guard.tryToRun() ) {
        printf("Another instance is already running...\n");
        return 0;
    }

    QApplication a(argc, argv);
    QCursor cursor(Qt::BlankCursor);
    QApplication::setOverrideCursor(cursor);
    QApplication::changeOverrideCursor(cursor);

    MainWindow w;
    w.show();
    return a.exec();
}

#include "mainwindow.h"
#include "runguard.h"
#include <QApplication>
#include <QDateTime>
#include <QByteArray>

void debugOut(QtMsgType type, const QMessageLogContext &context,
              const QString &msg)
{
    Q_UNUSED(context)

    QString full_msg;

    full_msg += QDateTime::currentDateTime().toString() + " : ";

    switch (type) {
    case QtDebugMsg:
        full_msg += "DEBUG : " + msg;
        break;
    case QtInfoMsg:
        full_msg += "INFO  : " + msg;
        break;
    case QtWarningMsg:
        full_msg += "WARN  : " + msg;
        break;
    case QtCriticalMsg:
        full_msg += "CRIT  : " + msg;
        break;
    case QtFatalMsg:
        full_msg += "FATAL : " + msg;
        break;
    }
    full_msg += "\n";

#ifdef DEBUG
    fprintf(stderr, "%s", full_msg.toLocal8Bit().constData());
#endif

    QFile logfile("logfile.txt");
    if (logfile.open(QFile::WriteOnly|QFile::Append)) {
        logfile.write(full_msg.toUtf8());
    }
}

int main(int argc, char *argv[])
{
    RunGuard guard( "wunderbar.com" );
    if ( !guard.tryToRun() ) {
        printf("Another instance is already running...\n");
        return 0;
    }

    qInstallMessageHandler(debugOut);

    QApplication a(argc, argv);
    QCursor cursor(Qt::BlankCursor);
    QApplication::setOverrideCursor(cursor);
    QApplication::changeOverrideCursor(cursor);

    a.setApplicationName("TestDisplay");
    a.setOrganizationName("wunderbar");
    a.setOrganizationDomain("wunderbar.com");
    MainWindow w;
    w.show();
    return a.exec();
}

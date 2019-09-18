#include <QObject>
#include <QRegularExpression>

#include "linuxprocessentry.h"

LinuxProcessEntry::LinuxProcessEntry(pid_t pid)
    : process_pid(pid)
{
    update();
}

LinuxProcessEntry::~LinuxProcessEntry()
{
//    qDebug() << Q_FUNC_INFO;
}

void LinuxProcessEntry::dump() const {
    qDebug() << "PID : " << process_pid << " PPID : " << parent_pid <<
                " PGID : " << group_pid << " STATE : " << process_state <<
                " CMD : " << process_cmd;
}

//(\d+)\s(\(.*\))\s(\w)\s(\d+)\s(\d+)\s(\d+)

bool LinuxProcessEntry::update()
{
    QFile   file;
    QString sFileName;

    sFileName = QString(PROC_DIRECTORY) + "/" +  QString::number(process_pid)+ "/cmdline";

    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        QString cmd;
        char    ch;

        while (file.getChar(&ch)) {
            if (file.atEnd()) {
                break;
            }
            if (ch == 0) {
                ch = ' ';
            }
            cmd += ch;
        }

        process_cmd = cmd;
        file.close();
    } else {
        qDebug() << "ERROR: Unable to open " << sFileName;
        return false;
    }

    sFileName = QString(PROC_DIRECTORY) + "/" +  QString::number(process_pid)+ "/stat";
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        QString statLine = file.readAll();

        QRegularExpression  exp("(\\d+)\\s(\\(.*\\))\\s(\\w)\\s(\\d+)\\s(\\d+)\\s(\\d+)");

//        qDebug() << statLine;

        QRegularExpressionMatch matches = exp.match(statLine);

        if (matches.hasMatch()) {
            process_state   = matches.captured(3).front();
            parent_pid      = matches.captured(4).toInt();
            group_pid       = matches.captured(5).toInt();
        }

        file.close();
    } else {
        qDebug() << "ERROR: Unable to open " << sFileName;
        return false;
    }

    return true;
}

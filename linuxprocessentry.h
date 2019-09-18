#ifndef PROCESSENTRY_H
#define PROCESSENTRY_H

#include <QFile>
#include <QDebug>
#include <QVector>
#include <QSharedPointer>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h> // for opendir(), readdir(), closedir()
#include <sys/stat.h> // for stat()

#define PROC_DIRECTORY      "/proc/"

class LinuxProcessEntry : public QObject {
    Q_OBJECT

public:
    LinuxProcessEntry(pid_t pid);
     ~LinuxProcessEntry();

    void            dump() const;

    pid_t           pid() const {
        return process_pid;
    }

    pid_t           ppid() const {
        return parent_pid;
    }

    pid_t           pgid() const {
        return group_pid;
    }

protected:
    bool            update();

    QString         process_cmd;
    QChar           process_state;

    pid_t           process_pid = -1;
    pid_t           parent_pid  = -1;
    pid_t           group_pid   = -1;
};

using LinuxProcessEntryPtr  = QSharedPointer<LinuxProcessEntry>;
using LinuxProcessVector    = QVector<LinuxProcessEntryPtr>;

//typedef QVector<LinuxProcessEntry *> LinuxProcessVector;

#endif // PROCESSENTRY_H

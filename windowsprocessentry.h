#ifndef PROCESSENTRY_H
#define PROCESSENTRY_H

#include <QFile>
#include <QDebug>
#include <QVector>
#include <QSharedPointer>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>

#define PROC_DIRECTORY      "/proc/"

class WindowsProcessEntry : public QObject {
    Q_OBJECT

public:
    WindowsProcessEntry(WORD pid);
     ~WindowsProcessEntry();

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

    WORD            process_pid = -1;
    WORD            parent_pid  = -1;
    WORD            group_pid   = -1;
};

using WindowsProcessEntryPtr  = QSharedPointer<WindowsProcessEntry>;
using WindowsProcessVector    = QVector<WindowsProcessEntryPtr>;

#endif // PROCESSENTRY_H

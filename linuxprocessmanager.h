#ifndef LINUXPROCESSMANAGER_H
#define LINUXPROCESSMANAGER_H

#include <QObject>
#include "linuxprocessentry.h"

class LinuxProcessManager : public QObject
{
    Q_OBJECT
public:
    explicit LinuxProcessManager(QObject *parent = nullptr);
    bool                    update();

    const LinuxProcessVector & getProcessVec() const;

    const LinuxProcessEntryPtr getProcessById(pid_t pid);

    bool getProcessesWithParent(pid_t ppid, LinuxProcessVector & vec);

signals:

public slots:

private:
    LinuxProcessVector      processVec;


};

#define PROC_DIRECTORY      "/proc/"

#endif // LINUXPROCESSMANAGER_H

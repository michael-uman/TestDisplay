#ifndef WINDOWSPROCESSMANAGER_H
#define WINDOWSPROCESSMANAGER_H

#include <QObject>
#include "windowsprocessentry.h"

class WindowsProcessManager : public QObject
{
    Q_OBJECT
public:
    explicit WindowsProcessManager(QObject *parent = nullptr);
    bool                    update();

    const WindowsProcessVector &    getProcessVec() const;
    const WindowsProcessEntryPtr    getProcessById(pid_t pid);
    bool                            getProcessesWithParent(pid_t ppid,
                                                           WindowsProcessVector & vec);

signals:

public slots:

private:
    WindowsProcessVector      processVec;


};

#endif // WINDOWSPROCESSMANAGER_H

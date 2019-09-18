#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <QVector>

#include <unistd.h>
#include <dirent.h>
#include <sys/types.h> // for opendir(), readdir(), closedir()
#include <sys/stat.h> // for stat()

#include "linuxprocessentry.h"
#include "linuxprocessmanager.h"

LinuxProcessManager::LinuxProcessManager(QObject *parent) : QObject(parent)
{
}

bool LinuxProcessManager::update()
{
    DIR * dir_proc = nullptr;

//    qDebug() << Q_FUNC_INFO;

    if ((dir_proc = opendir(PROC_DIRECTORY)) == nullptr) {
        perror("Couldn't open the " PROC_DIRECTORY " directory");
        return false;
    }

    struct dirent * entry = nullptr;

    while ((entry = readdir(dir_proc)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            QString entryName = entry->d_name;
            bool isNumeric = false;
            int procid = entryName.toInt(&isNumeric);
            if (isNumeric) {
                LinuxProcessEntryPtr pentry(new LinuxProcessEntry(procid));
                processVec.push_back(pentry);
            }
        }
    }

    closedir(dir_proc);

//    qDebug() << "Found " << processVec.size() << " processes...";

    return true;

}

const LinuxProcessVector &LinuxProcessManager::getProcessVec() const
{
    return processVec;
}

const LinuxProcessEntryPtr LinuxProcessManager::getProcessById(pid_t pid)
{
    LinuxProcessEntryPtr procEntry;

    if (processVec.length() > 0) {
        for (LinuxProcessEntryPtr process : processVec) {
            if (process->pid() == pid) {
                return process;
            }
        }
    }

    return procEntry;
}

bool LinuxProcessManager::getProcessesWithParent(pid_t ppid, LinuxProcessVector &vec)
{
    vec.clear();
    if (processVec.length() > 0) {
        for (LinuxProcessEntryPtr process : processVec) {
            if (process->ppid() == ppid) {
                vec.push_back(process);
            }
        }
    }

    return (vec.size() > 0);
}

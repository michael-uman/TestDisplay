#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <QVector>

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <tlhelp32.h>

#include "windowsprocessentry.h"
#include "windowsprocessmanager.h"

WindowsProcessManager::WindowsProcessManager(QObject *parent) : QObject(parent)
{
}

void PrintProcessNameAndID( DWORD processID )
{
    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

    // Get a handle to the process.

    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ,
                                   FALSE, processID );

    // Get the process name.

    if (NULL != hProcess )
    {
        HMODULE hMod;
        DWORD cbNeeded;

        if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod),
             &cbNeeded) )
        {
            MODULEINFO moduleInfo = {};

            GetModuleBaseName( hProcess, hMod, szProcessName,
                               sizeof(szProcessName)/sizeof(TCHAR) );

            GetModuleInformation( hProcess, hMod, &moduleInfo, sizeof(moduleInfo));

            qDebug() << "OK";

            pNtQuery
        }
    }

    QString procName = QString::fromWCharArray(szProcessName);

    // Print the process name and identifier.

    _tprintf( TEXT("%s  (PID: %u)\n"), procName.toLatin1().data(), processID );

    // Release the handle to the process.

    CloseHandle( hProcess );
}

bool WindowsProcessManager::update()
{
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
    {
        return 1;
    }

    // Calculate how many process identifiers were returned.

    cProcesses = cbNeeded / sizeof(DWORD);

    // Print the name and process identifier for each process.

    for ( i = 0; i < cProcesses; i++ )
    {
        if( aProcesses[i] != 0 )
        {
            PrintProcessNameAndID( aProcesses[i] );
        }
    }

    return true;

}

const WindowsProcessVector &WindowsProcessManager::getProcessVec() const
{
    return processVec;
}

const WindowsProcessEntryPtr WindowsProcessManager::getProcessById(pid_t pid)
{
    WindowsProcessEntryPtr procEntry;

    if (processVec.length() > 0) {
        for (WindowsProcessEntryPtr process : processVec) {
            if (process->pid() == pid) {
                return process;
            }
        }
    }

    return procEntry;
}

bool WindowsProcessManager::getProcessesWithParent(pid_t ppid, WindowsProcessVector &vec)
{
    vec.clear();
    if (processVec.length() > 0) {
        for (WindowsProcessEntryPtr process : processVec) {
            if (process->ppid() == ppid) {
                vec.push_back(process);
            }
        }
    }

    return (vec.size() > 0);
}

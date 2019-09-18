#include <QObject>
#include <QRegularExpression>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>

#include "WindowsProcessEntry.h"

WindowsProcessEntry::WindowsProcessEntry(WORD pid)
    : process_pid(pid)
{
    update();
}

WindowsProcessEntry::~WindowsProcessEntry()
{
//    qDebug() << Q_FUNC_INFO;
}

void WindowsProcessEntry::dump() const {
    qDebug() << "PID : " << process_pid << " PPID : " << parent_pid <<
                " PGID : " << group_pid << " STATE : " << process_state <<
                " CMD : " << process_cmd;
}

//(\d+)\s(\(.*\))\s(\w)\s(\d+)\s(\d+)\s(\d+)

bool WindowsProcessEntry::update()
{
    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

    // Get a handle to the process.

    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ,
                                   FALSE, process_pid );

    // Get the process name.

    if (NULL != hProcess )
    {
        HMODULE hMod;
        DWORD cbNeeded;

        if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod),
             &cbNeeded) )
        {
            GetModuleBaseName( hProcess, hMod, szProcessName,
                               sizeof(szProcessName)/sizeof(TCHAR) );
        }
    }

    QString procName = QString::fromWCharArray(szProcessName);

    // Print the process name and identifier.

    _tprintf( TEXT("%s  (PID: %u)\n"), procName.toLatin1().data(), process_pid );

    // Release the handle to the process.

    CloseHandle( hProcess );

    return true;
}

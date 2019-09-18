#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#if defined(__linux__)
#include "linuxprocessentry.h"
#include "linuxprocessmanager.h"

typedef LinuxProcessEntry       ProcessEntry;
typedef LinuxProcessVector      ProcessVector;
typedef LinuxProcessManager     ProcessManager;
typedef LinuxProcessEntryPtr    ProcessEntryPtr;

#elif defined(__WIN32__)
#include "windowsprocessentry.h"
#include "windowsprocessmanager.h"

typedef WindowsProcessEntry     ProcessEntry;
typedef WindowsProcessVector    ProcessVector;
typedef WindowsProcessManager   ProcessManager;
typedef WindowsProcessEntryPtr  ProcessEntryPtr;

#endif

#endif // PROCESSMANAGER_H


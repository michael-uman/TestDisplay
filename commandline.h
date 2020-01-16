#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <QVector>
#include <QString>
#include <QStringList>

class MainWindow;

class CommandInfo {
public:
    QStringList         args;
    QString             response = "OK";
};

typedef bool (MainWindow::*action)(CommandInfo & info);

typedef struct _commandEntry {
    QString     command;
    int         minOptions;
    int         maxOptions;
    action      callback;
} CMDENTRY;


#endif // COMMANDLINE_H

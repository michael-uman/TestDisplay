#ifndef DATABASELOGGER_H
#define DATABASELOGGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDebug>
#include <QDateTime>

class DatabaseLogger : public QObject
{
    Q_OBJECT

    QSqlDatabase            db;

public:
    explicit    DatabaseLogger(QObject *parent = nullptr);
    virtual     ~DatabaseLogger();

    bool        open();
    void        close();

signals:

public slots:
    void        handleLog(QString type, QString payload);
};

#endif // DATABASELOGGER_H

#ifndef DATABASELOGGER_H
#define DATABASELOGGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDebug>
#include <QDateTime>

//#define DEBUG_DB

class DatabaseLogger : public QObject
{
    Q_OBJECT

    QSqlDatabase            db;
    int                     lastStartId = -1;

public:
    explicit    DatabaseLogger(QObject *parent = nullptr);
    virtual     ~DatabaseLogger();

    bool        open();
    void        close();

    inline bool isOpen() const {
        return db.isOpen();
    }

    QString     getRecentTable();

signals:

public slots:
    void        handleLog(QString type, QString payload, int exitCode);
};

#endif // DATABASELOGGER_H

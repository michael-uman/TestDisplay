#include <QDebug>
#include <QSqlError>
#include "databaselogger.h"

#define PSQL_USERNAME       "developer"
#define PSQL_DBNAME         "developer"
#define PSQL_HOSTNAME       "wunderbar-ub18-dev"
#define PSQL_PASSWD         "realmagic"

DatabaseLogger::DatabaseLogger(QObject *parent)
    : QObject(parent)
{
}

DatabaseLogger::~DatabaseLogger()
{
    close();
}

bool DatabaseLogger::open()
{
    bool result = false;

    db = QSqlDatabase::addDatabase("QPSQL", "psql");

    if (db.isValid()) {
        db.setUserName(PSQL_USERNAME);
        db.setPassword(PSQL_DBNAME);
        db.setHostName(PSQL_HOSTNAME);
        db.setPassword(PSQL_PASSWD);

        if (db.open()) {
            qDebug() << "Database open";
            result = true;
        }
    }

    return result;
}

void DatabaseLogger::close()
{
    if (db.isOpen()) {
        db.close();
        qDebug() << "database closed";
    }
}

void DatabaseLogger::handleLog(QString type, QString payload)
{
    QSqlQuery q(db);

    qDebug() << Q_FUNC_INFO << type << payload;

    q.prepare("INSERT INTO event (event_type, event_payload) VALUES (?, ?)");
    q.bindValue(0, type);
    q.bindValue(1, payload);

    if (!q.exec()) {
        qDebug() << "Error adding event" << q.lastError();
    }
}

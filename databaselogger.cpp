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
#ifdef DEBUG_DB
            qDebug() << "Database open";
#endif
            result = true;
        }
    }

    return result;
}

void DatabaseLogger::close()
{
    if (db.isOpen()) {
        db.close();
#ifdef DEBUG_DB
        qDebug() << "database closed";
#endif
    }
}

void DatabaseLogger::handleLog(QString type, QString payload)
{
    QSqlQuery q(db);

#ifdef DEBUG_DB
    qDebug() << Q_FUNC_INFO << type << payload;
#endif

    q.prepare("INSERT INTO event (event_type, event_payload) VALUES (?, ?)");
    q.bindValue(0, type);
    q.bindValue(1, payload);

    if (!q.exec()) {
        qWarning() << "Error adding event" << q.lastError();
    }
}

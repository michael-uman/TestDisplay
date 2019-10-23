#include <QDebug>
#include <QSqlError>
#include <QHostInfo>
#include <QMutexLocker>

#include "databaselogger.h"

#define PSQL_USERNAME       "developer"
#define PSQL_DBNAME         "developer"
#define PSQL_HOSTNAME       "wunderbar-ub18-dev"
#define PSQL_PASSWD         "realmagic"

DatabaseLogger::DatabaseLogger(QObject *parent)
    : QObject(parent),
      dbMutex(QMutex::Recursive)
{
}

DatabaseLogger::~DatabaseLogger()
{
    close();
}

bool DatabaseLogger::open()
{
    QMutexLocker locker(&dbMutex);

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
    QMutexLocker locker(&dbMutex);

    if (db.isOpen()) {
        db.close();
#ifdef DEBUG_DB
        qDebug() << "database closed";
#endif
    }
}

QString DatabaseLogger::getRecentTable()
{
    QMutexLocker locker(&dbMutex);

    QString sTable;
    QSqlQuery q(db);

    // Only list logs from this host...
    q.prepare("SELECT * FROM (SELECT * FROM event WHERE event_host = ? ORDER BY event_ts DESC LIMIT 6) AS a ORDER BY event_ts ASC");
    q.bindValue(0, QHostInfo::localHostName());

    if (!q.exec()) {
        qDebug() << "Error in query!";
    }

    sTable += "<table border=\"1\" style=\"font-family:monospace; font-size: 12pt;\">\n";
    sTable += "  <tr><th>ID</th><th>Date/Time</th><th>Type</th><th>Payload</th><th>Ref</th></tr>\n";
    while (q.next()) {
        QSqlRecord  record = q.record();
        int         eventId, eventRef;
        QDateTime   eventTS;
        QString     eventType;
        QString     eventPayload;
//        int         eventCode;

        eventId         = record.value("event_id").toInt();
        eventTS         = record.value("event_ts").toDateTime();
        eventType       = record.value("event_type").toString();
        eventPayload    = record.value("event_payload").toString();
//        eventCode       = record.value("event_code").toInt();
        eventRef        = record.value("event_ref").toInt();

        sTable += "  <tr><td>" + QString::number(eventId) + "</td><td>" + eventTS.toString() +
                "</td><td>" + eventType + "</td><td>" + eventPayload + "</td><td>" +
                QString::number(eventRef) + "</td></tr>";

    }
    sTable += "</table>\n";

    return sTable;
}

void DatabaseLogger::handleLog(QString type, QString payload, int exitCode)
{
    QMutexLocker    locker(&dbMutex);
    QSqlQuery       q(db);

#ifdef DEBUG_DB
    qDebug() << Q_FUNC_INFO << type << payload;
#endif

    q.prepare("INSERT INTO event (event_type, event_payload, event_ref, event_host, event_code) VALUES (?, ?, ?, ?, ?)");

    if (type.compare("start", Qt::CaseInsensitive) == 0) {
        lastStartId = 0;
    }

    q.bindValue(0, type);
    q.bindValue(1, payload);
    q.bindValue(2, (lastStartId != -1)?lastStartId:0);
    q.bindValue(3, QHostInfo::localHostName());
    q.bindValue(4, exitCode);

    if (!q.exec()) {
        qWarning() << "Error adding event" << q.lastError();
    }

    if (type.compare("start", Qt::CaseInsensitive) == 0) {
        lastStartId = q.lastInsertId().toInt();
    }
//    else if (type.compare("stop", Qt::CaseInsensitive) == 0) {
//        lastStartId = -1;
//    }
}

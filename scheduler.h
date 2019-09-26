#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <QObject>
#include <QVector>
#include <QTimer>
#include <QDomDocument>
#include <QFileSystemWatcher>

/**
 * Enumeration of day of week.
 */

enum DOW {
    DOW_SUNDAY      = 1L << 0,
    DOW_MONDAY      = 1L << 1,
    DOW_TUESDAY     = 1L << 2,
    DOW_WEDNESDAY   = 1L << 3,
    DOW_THURSDAY    = 1L << 4,
    DOW_FRIDAY      = 1L << 5,
    DOW_SATURDAY    = 1L << 6,
};

struct eventTimeStamp {
    int         dow     = 0;
    int         hour    = 0;
    int         min     = 0;
};

class Scheduler;

/**
 * Scheduled Event class.
 */

class ScheduleEvent : public QObject
{
    Q_OBJECT
public:
    ScheduleEvent();
    ScheduleEvent(QString name, QObject * parent = nullptr);
    ScheduleEvent(const ScheduleEvent & copy);

    virtual ~ScheduleEvent();

    void            setName(QString name);
    void            setDescription(QString desc);
    void            setCommand(QString cmd);

    bool            scheduleEvent(qint32 dow, int hour, int min);

    void            dump();

protected:
    friend class Scheduler;

    QString         eventName;
    QString         eventDesc;
    QString         eventCommand;

    eventTimeStamp  eventTime;
};


using ScheduleEventVector = QVector<ScheduleEvent *>;

/**
 * @brief The Scheduler class
 */

class Scheduler : public QObject
{
    Q_OBJECT
public:
    explicit                Scheduler(QObject *parent = nullptr);
    virtual                 ~Scheduler();

    bool                    scheduleEvent(QString name, QString desc,
                                          QString cmd, qint32 dow,
                                          int hour, int min);
    bool                    scheduleEvent(const ScheduleEvent & event);

    void                    dump();

    int                     size() const {
        return events.size();
    }

    bool                    loadSchedule(const QString & scheduleName);
    bool                    saveSchedule(QString & scheduleName);

    bool                    watchFile(QString & file);

signals:
    void                    runCommand(QString command);

public slots:
    void                    timeout();
    void                    fileChanged(const QString & path);

protected:
    QTimer *                timer   = nullptr;
    ScheduleEventVector     events;
    QFileSystemWatcher      watcher;

    bool                    initTimer();
    void                    releaseTimer();

private:
    bool                    isDOWSet(const QDate & date, int dowMask);
    bool                    parseDateTime(const QDomNode & node,
                                          int & dowMask,
                                          int & hour, int & min);
};

#endif // SCHEDULER_H

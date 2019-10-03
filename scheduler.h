#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <QDebug>
#include <QObject>
#include <QVector>
#include <QTimer>
#include <QDomDocument>
#include <QFileSystemWatcher>
#include <QSharedPointer>

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

class Scheduler;
class ScheduleEvent;

class hourmin : public QObject {
    Q_OBJECT

public:
    hourmin() : QObject(nullptr), hour(0), min(0) {}
    hourmin(int h, int m) : QObject(nullptr), hour(h), min(m) {}
    hourmin(const hourmin & copy) : QObject(nullptr), hour(copy.hour), min(copy.min) {}

    hourmin & operator =(const hourmin & copy) {
        hour = copy.hour;
        min = copy.min;

        return *this;
    }

    int hour;
    int min;
};

class eventTimeRef  {
public:
    explicit eventTimeRef() {}
    ~eventTimeRef() {}

    void setDays(int dowMask) {
        dow = dowMask;
    }
    void addTime(int hour, int min) {
        hourmin hm(hour,min);
        times.append(hm);
    }

    static bool doEventsConflict(const eventTimeRef & a, const eventTimeRef & b);

protected:
    friend class ScheduleEvent;
    friend class Scheduler;

    int         dow     = 0;

    QVector<hourmin>    times;
};

/**
 * Scheduled Event class.
 */

class ScheduleEvent : public QObject
{
    Q_OBJECT
public:
    ScheduleEvent(QString name, QObject * parent = nullptr);
    ScheduleEvent(const ScheduleEvent & copy);

    virtual ~ScheduleEvent();

    bool            valid() const;

    void            setName(QString name);
    void            setDescription(QString desc);
    void            setCommand(QString cmd);
    void            setDOW(int dowMask);

    bool            scheduleEvent(const hourmin & hm);
    bool            scheduleEvent(int hour, int min);
    bool            scheduleEvent(const QString & timestr);

    void            dump();
    static bool     doEventsConflict(const ScheduleEvent & a, const ScheduleEvent & b);

    bool            isEventTime(const QDateTime & ts);

protected:
    friend class Scheduler;

    QString         eventName;
    QString         eventDesc;
    QString         eventCommand;

    eventTimeRef    eventTime;
};

using ScheduleEventPtr    = QSharedPointer<ScheduleEvent>;
using ScheduleEventVector = QVector<ScheduleEventPtr>;

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

    void                    enable(bool enabled);
    bool                    isEnabled() const;

signals:
    void                    runCommand(QString command);

public slots:
    void                    timeout();
    void                    fileChanged(const QString & path);

protected:
    QTimer *                timer   = nullptr;
    ScheduleEventVector     events;
    QFileSystemWatcher      watcher;
    QAtomicInteger<bool>    bSchedulerEnabled = true;

    bool                    initTimer();
    void                    releaseTimer();

private:
    bool                    isDOWSet(const QDate & date, int dowMask);
    bool                    parseDateTime(const QDomNode & node,
                                          int & dowMask,
                                          int & hour, int & min);
    bool                    parseDateTime(const QDomNode & node, ScheduleEvent & event);
};


#endif // SCHEDULER_H

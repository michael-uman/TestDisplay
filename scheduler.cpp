#include <QDebug>
#include <QDateTime>
#include "scheduler.h"

const QString dow_string[] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"
};


ScheduleEvent::ScheduleEvent() : QObject(nullptr)
{

}

ScheduleEvent::ScheduleEvent(QString name, QObject * parent)
    : QObject(parent),
      eventName(name)
{

}

ScheduleEvent::ScheduleEvent(const ScheduleEvent &copy)
    : QObject(copy.parent()),
      eventName(copy.eventName),
      eventDesc(copy.eventDesc),
      eventCommand(copy.eventCommand),
      eventTime(copy.eventTime)
{
}

ScheduleEvent::~ScheduleEvent()
{

}

void ScheduleEvent::setName(QString name)
{
    eventName = name;
}

void ScheduleEvent::setDescription(QString desc)
{
    eventDesc = desc;
}

void ScheduleEvent::setCommand(QString cmd)
{
    eventCommand = cmd;
}

bool ScheduleEvent::scheduleEvent(qint32 dow, int hour, int min)
{
    eventTime.dow   = dow;
    eventTime.hour  = hour;
    eventTime.min   = min;

    return true;
}

void ScheduleEvent::dump()
{
    qDebug() << "Name : " << eventName;
    qDebug() << "Desc : " << eventDesc;
    qDebug() << "Cmd  : " << eventCommand;
    QString days;
    for (int i = 0 ; i < 7 ; i++) {
        if (eventTime.dow & (1L << i)) {
            if (!days.isEmpty()) {
                days += " ";
            }
            days += dow_string[i];
        }
    }
    qDebug() << "Days : " << days;
    QString time = QString::asprintf("%02d:%02d", eventTime.hour, eventTime.min);
    qDebug() << "Time : " << time;

    return;
}

bool Scheduler::isDOWSet(const QDate &date, int dowMask)
{
    int dow = date.dayOfWeek();

    if (dow == 7) {
        dow = 0;
    }

    return ((dowMask & (1L << dow)) != 0);
}

Scheduler::Scheduler(QObject *parent) : QObject(parent)
{
    qDebug() << Q_FUNC_INFO;
    initTimer();
}

Scheduler::~Scheduler()
{
    qDebug() << Q_FUNC_INFO;
    releaseTimer();
}

bool Scheduler::scheduleEvent(QString name, QString desc, QString cmd, qint32 dow, int hour, int min)
{
    for (const auto & ev : events) {
        if (((ev->eventTime.dow & dow) != 0) && (ev->eventTime.hour == hour) && (ev->eventTime.min == min)) {
            qDebug() << "Event times intersected";
            return false;
        }
    }

    ScheduleEvent     *  newEvent = new ScheduleEvent(name);
    newEvent->setDescription(desc);
    newEvent->setCommand(cmd);
    newEvent->scheduleEvent(dow, hour, min);

    events.push_back(newEvent);

    return true;
}

bool Scheduler::scheduleEvent(const ScheduleEvent &event)
{
    for (const auto & ev : events) {
        if (((ev->eventTime.dow & event.eventTime.dow) != 0) &&
             (ev->eventTime.hour == event.eventTime.hour) &&
             (ev->eventTime.min == event.eventTime.min))
        {
            qDebug() << "Event times intersected";
            return false;
        }
    }
    ScheduleEvent * newEvent = new ScheduleEvent(event);

    events.push_back(newEvent);

    return true;
}

void Scheduler::dump()
{
    qDebug() << "Dump of all events in scheduler:";

    for (const auto & ev : events) {
        ev->dump();
    }
}

bool Scheduler::loadSchedule(QString &scheduleName)
{
    qDebug() << Q_FUNC_INFO;
    Q_UNUSED(scheduleName)
    return false;
}

bool Scheduler::saveSchedule(QString &scheduleName)
{
    qDebug() << Q_FUNC_INFO;
    Q_UNUSED(scheduleName)
    return false;
}

void Scheduler::timeout()
{
    qDebug() << Q_FUNC_INFO;
    QDateTime   currentDateTime = QDateTime::currentDateTime();
    qDebug() << currentDateTime.toString();
    QTime       currentTime = currentDateTime.time();
    QDate       currentDate = currentDateTime.date();

    for (auto & event : events) {
        if (isDOWSet(currentDate, event->eventTime.dow)) {
//          qDebug() << "This event matches Day Of Week";
            if ((currentTime.hour() == event->eventTime.hour) &&
                (currentTime.minute() == event->eventTime.min) &&
                (currentTime.second() == 0))
            {
                qInfo() << "Event " << event->eventName << " has occured";
                emit runCommand(event->eventCommand);
            }
        }
    }

}

bool Scheduler::initTimer()
{
    qDebug() << Q_FUNC_INFO;
    timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this, &Scheduler::timeout);

    timer->start(1000);

    qInfo() << "Timer started";

    return true;
}

void Scheduler::releaseTimer()
{
    qDebug() << Q_FUNC_INFO;

    timer->stop();
    delete timer;
    timer = nullptr;

    qInfo() << "Timer stopped";
}

#include <QDebug>
#include <QDateTime>
#include <QDomDocument>
#include <QFile>
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

extern const QString settings_path;

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

bool Scheduler::parseDateTime(const QDomNode &node, int &dowMask, int &hour, int &min)
{
    for (QDomNode n = node ; !n.isNull() ; n = n.nextSibling()) {
        if (n.nodeType() == QDomNode::ElementNode) {
            QDomElement elem = n.toElement();

            if (elem.tagName() == "dow") {
                QString dowString = elem.text();
                if (dowString.length() == 7) {
                    for (int i = 0 ; i < 7 ; i++) {
                        if (dowString[i] == "1") {
                            dowMask |= (1L << i);
                        }
                    }
                } else {
                    qWarning() << "Invalid dow field!";
                }
            } else if (elem.tagName() == "hour") {
                hour = elem.text().toInt();
            } else if (elem.tagName() == "min") {
                min = elem.text().toInt();
            } else {
                qWarning() << "Unknown tag " << elem.tagName();
            }
        }
    }

    return true;
}

Scheduler::Scheduler(QObject *parent)
    : QObject(parent),
      watcher(parent)
{
    qDebug() << Q_FUNC_INFO;
    initTimer();

    connect(&watcher, &QFileSystemWatcher::fileChanged, this, &Scheduler::fileChanged);
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

bool Scheduler::loadSchedule(const QString &scheduleName)
{
    qDebug() << Q_FUNC_INFO;
    Q_UNUSED(scheduleName)

    QDomDocument        doc;
    QFile               xmlFile(scheduleName);

    if (!xmlFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to open schedule XML file";
        return false;
    }

    if (!doc.setContent(&xmlFile)) {
        qWarning() << "Error reading schedule XML file";
        xmlFile.close();
        return false;
    }
    QDomElement root = doc.documentElement();

    if (root.tagName() != "schedule") {
        qWarning() << "Not a schedule XML file";
        return false;
    }

    events.clear();

    for (QDomNode n = root.firstChild() ; !n.isNull() ; n = n.nextSibling()) {
        QDomElement domElem = n.toElement();

        if (domElem.tagName() == "event") {
            QString eventName = domElem.attribute("name");
            QString eventDesc, eventCommand;
            int dowMask = 0, hour = 0, min = 0;

            for (QDomNode s = n.firstChild() ; !s.isNull() ; s = s.nextSibling()) {
                if (s.nodeType() != QDomNode::ElementNode)
                    continue;

                domElem = s.toElement();
                if (domElem.tagName() == "datetime") {

                    parseDateTime(domElem.firstChild(), dowMask, hour, min);

                } else if (domElem.tagName() == "desc") {
                    eventDesc = domElem.text();
                } else if (domElem.tagName() == "command") {
                    eventCommand = domElem.text();
                } else {
                    qWarning() << "Unknown tag " << domElem.tagName();
                }
            }

            qInfo() << "Adding " << eventName << " desc " << eventDesc << " cmd " << eventCommand;

            scheduleEvent(eventName, eventDesc, eventCommand, dowMask, hour, min);
        }
    }

    return true;
}

bool Scheduler::saveSchedule(QString &scheduleName)
{
    qDebug() << Q_FUNC_INFO;
    Q_UNUSED(scheduleName)
    return false;
}

bool Scheduler::watchFile(QString & filename)
{
    qDebug() << Q_FUNC_INFO << "file" << filename;

    return watcher.addPath(filename);
}

void Scheduler::timeout()
{
    QDateTime   currentDateTime = QDateTime::currentDateTime();
    QTime       currentTime = currentDateTime.time();
    QDate       currentDate = currentDateTime.date();

    // Run through all events checking if the time has come...
    for (auto & event : events) {
        if (isDOWSet(currentDate, event->eventTime.dow)) {
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

void Scheduler::fileChanged(const QString &path)
{
    qDebug() << Q_FUNC_INFO << " path " << path;
    loadSchedule(path);
    return;
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


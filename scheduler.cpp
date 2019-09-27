#include <QDebug>
#include <QDateTime>
#include <QDomDocument>
#include <QFile>
#include <QTime>
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

bool eventTimeRef::doEventsConflict(const eventTimeRef &a, const eventTimeRef &b)
{
    if ((a.dow & b.dow) != 0) {
        for (const auto & timea : a.times) {
            for (const auto & timeb : b.times) {
                if ((timea.hour == timeb.hour) &&
                    (timea.min == timeb.min))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

ScheduleEvent::ScheduleEvent()
    : QObject(nullptr)
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

bool ScheduleEvent::valid() const
{
    bool bValid = false;

    if (!eventName.isEmpty() && !eventDesc.isEmpty() && !eventCommand.isEmpty()) {
        if (eventTime.dow != 0) {
            bValid = true;
        }
    }

    return bValid;
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

void ScheduleEvent::setDOW(int dowMask)
{
    eventTime.dow = dowMask;
}

bool ScheduleEvent::scheduleEvent(const hourmin &hm)
{
    eventTime.times.append(hm);
    return true;
}

bool ScheduleEvent::scheduleEvent(int hour, int min)
{
    hourmin schedTime(hour, min);
    eventTime.times.append(schedTime);

    return true;
}

bool ScheduleEvent::scheduleEvent(const QString &timestr)
{
    QTime       ts(QTime::fromString(timestr, "hh:mmAP"));
    hourmin     schedTime(ts.hour(), ts.minute());
    eventTime.times.append(schedTime);
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

    for (const auto & tr : eventTime.times) {
        QString time = QString::asprintf("%02d:%02d", tr.hour, tr.min);
        qDebug() << "Time : " << time;
    }

    qDebug() << "Vali : " << QString(valid()?"True":"False");

    return;
}

bool ScheduleEvent::doEventsConflict(const ScheduleEvent &a, const ScheduleEvent &b)
{
    return eventTimeRef::doEventsConflict(a.eventTime, b.eventTime);
}

bool ScheduleEvent::isEventTime(const QDateTime &ts)
{
    int dow  = ts.date().dayOfWeek(); //  date.dayOfWeek();
    int hour = ts.time().hour();
    int min  = ts.time().minute();
    int sec  = ts.time().second();

    if (sec != 0)
        return false;

    if (dow == 7) {
        dow = 0;
    }

    if ((eventTime.dow & (1L << dow)) != 0) {
        for (const auto & eventTS : eventTime.times) {
            if ((hour == eventTS.hour) &&
                (min == eventTS.min))
            {
                return true;
            }
        }
    }

    return false;
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
    // Check for conflicts
    for (const auto & ev : events) {
        if ((ev->eventTime.dow & dow) != 0) {
            for (const auto & ts : ev->eventTime.times) {
                if ((ts.hour == hour) && (ts.min == min))
                    return false;
            }
        }
    }

    ScheduleEvent     *  newEvent = new ScheduleEvent(name);

    newEvent->setDescription(desc);
    newEvent->setCommand(cmd);
    newEvent->setDOW(dow);
    newEvent->scheduleEvent(hour, min);

    events.push_back(newEvent);

    return true;
}

bool Scheduler::scheduleEvent(const ScheduleEvent &event)
{
    // Check for conflict
    for (const auto & ev : events) {
        if ((ev->eventTime.dow & event.eventTime.dow) != 0) {
            for (const auto & ts1 : ev->eventTime.times) {
                for (const auto & ts2 : event.eventTime.times) {
                    if ((ts1.hour == ts2.hour) && (ts1.min == ts2.min)) {
                        return false;
                    }
                }
            }
        }
    }

    ScheduleEvent * newEvent = new ScheduleEvent(event);

    events.push_back(newEvent);

    return true;
}

/**
 * Dump the schedule to debug.
 */
void Scheduler::dump()
{
    qDebug() << "Dump of all events in scheduler:";

    for (const auto & ev : events) {
        ev->dump();
    }
}

/**
 * Load a schedule from an XML file.
 */
bool Scheduler::loadSchedule(const QString &scheduleName)
{
    QDomDocument        doc;
    QFile               xmlFile(scheduleName);

#ifdef DEBUG
    qDebug() << Q_FUNC_INFO;
#endif

    qInfo() << "Loading schedule " << scheduleName;

    if (!xmlFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to open schedule XML file";
        return false;
    }

    if (!doc.setContent(&xmlFile)) {
        qWarning() << "Error reading schedule XML file";
        xmlFile.close();
        return false;
    }

    xmlFile.close();

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
            ScheduleEvent newEvent(eventName);

            for (QDomNode s = n.firstChild() ; !s.isNull() ; s = s.nextSibling()) {
                if (s.nodeType() != QDomNode::ElementNode)
                    continue;

                domElem = s.toElement();
                if (domElem.tagName() == "datetime") {
                    parseDateTime(domElem.firstChild(), newEvent);
                } else if (domElem.tagName() == "desc") {
                    newEvent.setDescription(domElem.text());
                } else if (domElem.tagName() == "command") {
                    newEvent.setCommand(domElem.text());
                } else {
                    qWarning() << "Unknown tag " << domElem.tagName();
                }
            }

            scheduleEvent(newEvent);
        }
    }

#ifdef DEBUG
    dump();
#endif

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
#ifdef DEBUG
    qDebug() << Q_FUNC_INFO << "file" << filename;
#endif
    return watcher.addPath(filename);
}

void Scheduler::timeout()
{
    QDateTime   currentDateTime = QDateTime::currentDateTime();

    // Run through all events checking if the time has come...
    for (auto & event : events) {
        if (event->isEventTime(currentDateTime)) {
            qInfo() << "Event " << event->eventName << " has occured";
            emit runCommand(event->eventCommand);
        }
    }
}

/**
 * When the schedule file is changed, reload the schedule xml.
 */
void Scheduler::fileChanged(const QString &path)
{
#ifdef DEBUG
    qDebug() << Q_FUNC_INFO << " path " << path;
#endif
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

bool Scheduler::isDOWSet(const QDate &date, int dowMask)
{
    int dow = date.dayOfWeek();

    if (dow == 7) {
        dow = 0;
    }

    return ((dowMask & (1L << dow)) != 0);
}

bool Scheduler::parseDateTime(const QDomNode &node, ScheduleEvent & event) {
    for (QDomNode n = node ; !n.isNull() ; n = n.nextSibling()) {
        if (n.nodeType() == QDomNode::ElementNode) {
            QDomElement elem = n.toElement();

            if (elem.tagName() == "dow") {
                QString dowString = elem.text();
                if (dowString.length() == 7) {
                    for (int i = 0 ; i < 7 ; i++) {
                        if (dowString[i] == "1") {
                            event.eventTime.dow |= (1L << i);
                        }
                    }
                } else {
                    qWarning() << "Invalid dow field!";
                }
            } else if (elem.tagName() == "time") {
                event.scheduleEvent(elem.text());
            } else {
                qWarning() << "Unknown tag " << elem.tagName();
            }
        }
    }

    return true;
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

#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QTcpSocket>
#include <QDate>
#include <QCoreApplication>
#include <QDir>
#include <QHostInfo>
#include <QSharedPointer>
#ifdef __linux__
    #include <signal.h>
    #include <sys/types.h>
#endif
#include "mainwindow.h"
#include "displayxmlparser.h"
#include "processmanager.h"
#include "testdisplayrequesthandler.h"
#include "databaselogger.h"

QStringList MainWindow::optionFilenames = {
    ".testdisplay/styles.xml",
    ".testdisplay/scripts.xml",
    ".testdisplay/schedule.xml"
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      mainMutex(QMutex::Recursive),
      appSettings(new QSettings()),
      dblog(this)
{
    appPid = QCoreApplication::applicationPid();
    qDebug() << "Application process id = " << appPid;

#ifdef DEBUG
    QSize WinSize = size();
    qDebug() << "Window Size = " << WinSize;
#endif

    if (!StartServer()) {
        qDebug() << "Exiting application...";
        close();
        return;
    }

    setAttribute(Qt::WA_AcceptTouchEvents);
    QMainWindow::showFullScreen();

    loadStyles();
    // If there are no styles in the vector, create a default style for use in mean time...
    if (styleVec.length() > 0) {
        style = styleVec[0];
    } else {
        DisplayStylePtr defaultStyle(new DisplayStyle("Default", QColor("yellow"), QColor("blue")));
        styleVec.push_back(defaultStyle);
        style = defaultStyle;
    }

    loadScripts();

    connect(&bgProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(processComplete(int, QProcess::ExitStatus)));

    /* Load schedule */
    QString sched_path = getOptionPath(optionFile::OPTION_SCHEDULE);

    sched.watchFile(sched_path);

    qDebug() << "Loading schedule from file...";
    if (!sched.loadSchedule(sched_path)) {
        qWarning() << "Unable to load schedule!";
    }

    connect(&sched, &Scheduler::runCommand, this, &MainWindow::runCommand);

    appSettings->beginGroup("settings");
    sched.enable(appSettings->value("scheduler", QVariant::fromValue(false)).toBool());
    appSettings->endGroup();

    sDateTime = QDateTime::currentDateTime().toString();

    state    = displayState::DISPLAY_MAIN;
    bRunning = true;

    repaint();

    updateTimer.setInterval(250);
    connect(&updateTimer, &QTimer::timeout, this, &MainWindow::onTimer);
    updateTimer.start();

    if (dblog.open()) {
        connect(this, &MainWindow::logMessage, &dblog, &DatabaseLogger::handleLog);
    } else {
        qWarning() << "Unable to open database for logging";
    }

    // Start HTTP Server...
    appSettings->beginGroup("listener");
    qDebug() << "Settings file @ " << appSettings->fileName();
    reqHandler = new TestDisplayRequestHandler(this);
    httpListener = new stefanfrings::HttpListener(appSettings.data(), reqHandler);
    appSettings->endGroup();
}

MainWindow::~MainWindow()
{
    dblog.close();
}

QSize MainWindow::getTextSize(QString text, QPainter & p) {
    QFontMetrics fm = p.fontMetrics();
    int tw = fm.width(text);
    int th = fm.height();
    return QSize(tw, th);
}

/**
 * Draw the main test display window.
 */

void MainWindow::paintMainDisplay()
{
    QString     sText;
    QSize       size(this->size());
    int         sw(size.width());
    int         sh(size.height());
    QPainter    p(this);
    QBrush      bgBrush(style->GetBgColor());
    QSize       txtSize;

    // Fill the window with the background color
    p.fillRect(0, 0, sw, sh, bgBrush);

    // Draw the text in white
    p.setPen(style->GetFgColor());

    // Draw the heading horizontally centered at the top of the window
    p.setFont(style->GetHeadingFont());
    txtSize = getTextSize(sHeading, p);
    p.drawText((sw - txtSize.width())/2, 96, sHeading);

    // Draw the message in the center of the window
    p.setFont(style->GetMessageFont());
    txtSize = getTextSize(sMessage, p);
    p.drawText((sw - txtSize.width())/2, (sh - txtSize.height())/2, sMessage);

    // Draw date/time
    if (bTimeEnabled) {
        p.setFont(QFont("Ubuntu Light", 22));
        txtSize = getTextSize(sDateTime, p);
        p.drawText((sw - txtSize.width())/2, sh - 130, sDateTime);
    }

    p.setFont(QFont("Bitstream Vera Sans Mono", 12));
    p.drawText(64, sh - 24, "'Q' => EXIT / 'S' => STOP");

    sText = QString("v%1.%2.%3 © 2019-2020 Wunder-Bar, Inc.").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_BUILD);
    txtSize = getTextSize(sText, p);

    p.drawText(sw - 70 - txtSize.width(), sh - 24, sText);
}

void MainWindow::paintMenuDisplay()
{
    QString     sText;
    QSize       size(this->size());
    int         sw(size.width());
    int         sh(size.height());
    QPainter    p(this);
    QBrush      bgBrush(style->GetBgColor());
    QSize       txtSize;
    int         index = 0;
    // Fill the window with the background color
    p.fillRect(0, 0, sw, sh, bgBrush);

    sText = "Select Script to Run";
    p.setPen(QColor("white"));
    p.setFont(QFont("Bitstream Vera Sans", 36));
    txtSize = getTextSize(sText, p);
    p.drawText((sw - txtSize.width())/2, 64, sText);

    p.setFont(QFont("Bitstream Vera Sans", 24));

    for (auto script : scriptMgr.getVec()) {
        sText =  script->name();
        txtSize = getTextSize(sText, p);
        p.drawText((sw - txtSize.width())/2, sh - 300 + (index * (txtSize.height() + 2)), sText);

        index++;
    }
}

// #281a0d
//QColor bgColor(40, 26, 13);
/**
 * @brief Draw the main window
 * @param event
 */
void MainWindow::paintEvent(QPaintEvent * event)
{
    if (bRunning) {
        switch (state) {
        case displayState::DISPLAY_BLANK:
            break;
        case displayState::DISPLAY_MAIN:
            paintMainDisplay();
            break;
        case displayState::DISPLAY_MENU:
            paintMenuDisplay();
            break;
        case displayState::DISPLAY_DIALOG:
            break;
        case displayState::DISPLAY_TEST:
            break;
        }
    }

    QMainWindow::paintEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent * event)
{
#ifdef DEBUG
    qDebug() << Q_FUNC_INFO;
#endif

    if (event->key() == Qt::Key_Q) {
        close();
    } else if (event->key() == Qt::Key_D) {
        QString script_path = QDir::homePath() + QDir::separator() + ".testdisplay/launch-demo.sh";
        startBgProcess(script_path);
    } else if (event->key() == Qt::Key_M) {
        if (state == displayState::DISPLAY_MAIN) {
            state = displayState::DISPLAY_MENU;
        } else {
            state = displayState::DISPLAY_MAIN;
        }
    } else if (event->key() == Qt::Key_S) {
        if (bgRunning) {
            stopBgProcess();
        } else {
            qWarning() << "Attempted to stop a script but no script running...";
        }
    } else if (event->key() == Qt::Key_F1) {
        qDebug() << "F1 Key Hit";
    } else {
        if (!bgRunning) {
            TestScriptPtr scriptInfo = scriptMgr.getScriptForKey(event->key());

            if (!scriptInfo.isNull()) {
                qDebug() << "Found script for key : " << scriptInfo->name();

                startBgProcess(scriptInfo->path());
            }
        } else {
            qWarning() << "Attempted to start a script but a script is already running...";
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *)
{
#ifdef DEBUG
    qDebug() << Q_FUNC_INFO;
#endif

    appSettings->beginGroup("settings");
    appSettings->setValue("scheduler", sched.isEnabled());
    appSettings->endGroup();

    StopServer();
    updateTimer.stop();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
#ifdef DEBUG
    qDebug() << Q_FUNC_INFO << event->size();
#else
    Q_UNUSED(event)
#endif
}

bool MainWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::TouchBegin:
        qDebug() << "touch!";

        mytouchEvent(static_cast<QTouchEvent *>(event));
        return true;
    default:
        // call base implementation
        return QMainWindow::event(event);
    }
}

void MainWindow::mytouchEvent(QTouchEvent *event)
{
    qDebug() << Q_FUNC_INFO;

    auto points = event->touchPoints();

    for (auto point : points) {
        qDebug() << point.pos();
    }

    QString script_path = QDir::homePath() + QDir::separator() + ".testdisplay/launch-demo.sh";
    startBgProcess(script_path);
}

/**
 * Start the TCP/IP Server listening to port
 */

bool MainWindow::StartServer()
{
#ifdef DEBUG
    qDebug() << Q_FUNC_INFO;
#endif

    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen(QHostAddress::Any, server_port)) {
        qDebug() << "Unable to open server on port!";
        return false;
    }

    connect(tcpServer, &QTcpServer::newConnection, this, &MainWindow::onConnection);

#ifdef DEBUG
    qDebug() << "Server opened on port " << SERVER_PORT;
#endif

    return true;
}

void MainWindow::StopServer()
{
#ifdef DEBUG
    qDebug() << Q_FUNC_INFO;
#endif
    tcpServer->close();
    delete tcpServer;

    tcpServer           = nullptr;
}

void MainWindow::resetText()
{
    sHeading        = DEFAULT_HEADING;
    sMessage        = DEFAULT_MESSAGE;
    bTimeEnabled    = true;
    repaint();
}

/**
 * Start the background process running, if it is already running kill the child process
 */

bool MainWindow::startBgProcess(QString script_path)
{
    bool bResult = false;

#ifdef DEBUG
    qDebug() << Q_FUNC_INFO;
#endif

    if (!bgRunning) {

        sMessage = "Starting script...";
        repaint();

        scriptLogFile = new QTemporaryFile(QDir::tempPath() + "/jt-log.XXXXXX");
        scriptLogFile->setAutoRemove(false);

        if (!scriptLogFile->open()) {
            qWarning() << "Unable to open temporary file";
        } else {
            qInfo() << "Scripts log file = " << scriptLogFile->fileName();
        }

        QStringList     script_args = {
            scriptLogFile->fileName(),
        };

        //QString script_path = QDir::homePath() + QDir::separator() + settings_path + QDir::separator() + "launch-test.sh";
        qInfo() << "Starting script @ " << script_path;

        bgProcess.start(script_path, script_args);

        if (bgProcess.waitForStarted()) {
            qInfo() << "Process Id : " << bgProcess.processId();
            runningScriptName = script_path;
            emit logMessage("START", script_path, 0);
            bgRunning = bResult = true;
        } else {
            qWarning() << "Script failed to start";
            runningScriptName.clear();
            sMessage = "Script failed to start!";
            bgRunning = bResult = false;
        }
    } else {
        qWarning() << "Process " << bgProcess.processId() << " already running...";
    }

    return bResult;
}

bool MainWindow::stopBgProcess()
{
    bool bResult = false;

    if (bgRunning) {
        ProcessManager      mgr;
        pid_t               pid = static_cast<pid_t>(bgProcess.processId());

        sMessage = "Stopping script...";
        repaint();

        if (mgr.update()) {
            ProcessVector procVec;

            if (mgr.getProcessesWithParent(pid, procVec)) {
                qInfo() << "Killing child process id " << pid;
#ifdef __linux__
                pid_t childPid = procVec[0]->pid();
                ::kill(childPid, SIGINT);
#else
                bgProcess.terminate();
#endif
                bResult = true;
            }
        }
    } else {
        qWarning() << "No background process is running...";
    }

    return bResult;
}

/**
 * Return the name of the option file based on it's enumerated value.
 */

QString MainWindow::getOptionPath(optionFile opt)
{
    if ((opt >= optionFile::OPTION_STYLES) && (opt <= optionFile::OPTION_SCHEDULE)) {
        QString fullPath = QDir::homePath() + QDir::separator() + optionFilenames.at(static_cast<int>(opt));
        return fullPath;
    }

    return QString();
}

bool MainWindow::loadStyles()
{
    QMutexLocker    locker(&mainMutex);

    // Read the styles from XML
    DisplayXmlParser parser(styleVec);
    QXmlSimpleReader reader;

    reader.setContentHandler(&parser);
    reader.setErrorHandler(&parser);

    QFile file(getOptionPath(optionFile::OPTION_STYLES));
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Unable to open file!";
        return false;
    } else {
        QXmlInputSource xmlSource(&file);
        if (!reader.parse(xmlSource)) {
            qWarning() << "Error parsing the XML file";
            return false;
        }
    }

    return true;
}

/**
 * Load user-invokable scripts from the XML file.
 *
 * \returns true on success, false on failure.
 */

bool MainWindow::loadScripts()
{
    QMutexLocker    locker(&mainMutex);
    bool            result = false;
    QString         xmlFilePath;

    xmlFilePath = getOptionPath(optionFile::OPTION_SCRIPTS);
    if (scriptMgr.load(xmlFilePath)) {
        qInfo() << "Scripts loaded";
        result = true;
    } else {
        qWarning() << "Error loading scripts";
    }
    return result;
}

/**
 *  Parsing the user supplied line.
 */
bool MainWindow::parseText(QString line, QString &response)
{
    bool            bResult     = false;
    QStringList     element     = line.split(':');
    int             splitsize   = element.length();

#ifdef DEBUG
    qDebug() << element;
#endif

    response = "OK";

    if (splitsize == 1) {
        setMessage(element[0]);
    } else if (splitsize == 2) {
        if (element[0] == "HEAD") {
            setHeading(element[1]);
        } else if (element[0] == "MESG") {
            setMessage(element[1]);
        } else if (element[0] == "QUIT") {
            bResult = true;
        } else if (element[0] == "TIME") {
            bTimeEnabled = (element[1] == "1");
        } else if (element[0] == "REST") {
            resetText();
        } else if (element[0] == "STYL") {
            int index = element[1].toInt();
            if (index < styleVec.length()) {
                style = styleVec[index];
            } else {
                response = "FAIL";
            }
        } else if (element[0] == "STAT") {
            response = get_status_string();
        } else if (element[0] == "RUNK") {
            if (element[1].length() == 1) {
                TestScriptPtr script = scriptMgr.getScriptForKey(element[1].front().toLatin1());
                if (!script.isNull()) {
                    if (!startBgProcess(script->path())) {
                        response = "FAIL";
                    }
                } else {
                    response = "FAIL";
                }
            } else {
                response = "FAIL";
            }
        } else if (element[0] == "STOP") {
            if (bgRunning) {
                stopBgProcess();
            } else {
                qWarning() << "No background process is running...";
                response = "FAIL";
            }
        } else if (element[0] == "LIST") {
            if (element[1] == "SCRIPT") {
                response = get_script_list();
            } else if (element[1] == "STYLE") {
                response = get_style_list();
            } else {
                qWarning() << "Invalid LIST type " << element[1];
                response = "FAIL";
            }
        } else if (element[0] == "KILL") {
            QKeyEvent quitEvent(QEvent::KeyPress, 'Q', Qt::NoModifier);
            QCoreApplication::sendEvent(this, &quitEvent);
        } else if (element[0] == "SCHED") {
            if (element[1] == "0") {
                sched.enable(false);
            } else if (element[1] == "1") {
                sched.enable(true);
            } else {
                qWarning() << "Invalid SCHED value " << element[1];
                response = "FAIL";
            }
        } else {
            qDebug() << "Unknown tag " << element[0];
            response = "FAIL";
        }
    } else if (splitsize == 3) {
        if (element[0] == "TEXT") {
            setHeading(element[1]);
            setMessage(element[2]);
        } else {
            response = "FAIL";
        }
    }

    return bResult;
}

QString MainWindow::get_status_string()
{
    QMutexLocker locker(&mainMutex);
    QString response;

    response = QString("STAT:");
#if 0
    response += QHostInfo::localHostName() + ":";
#endif
    response += sHeading + ":" + sMessage + ":";
    response += style->GetLabel() + ":";
    response += QString((bTimeEnabled == true)?"1":"0") + ":";
    response += QString((sched.isEnabled() == true)?"1":"0") + ":";
    response += QString((bgRunning == true)?"1":"0") + ":";
    response += runningScriptName;

    return response;
}

/*!
 * Return the list of scripts.
 *
 * \returns QString containing all script information.
 */

QString MainWindow::get_script_list()
{
    QMutexLocker    locker(&mainMutex);
    QString         result;
    QTextStream     stream(&result);

    if (scriptMgr.size() > 0) {
        for (const auto & script : scriptMgr.getVec()) {
            stream <<  script->key() << ":" << script->name() << ":" << script->path() << endl;
        }
        stream << "OK";
    } else {
        stream << "OK";
    }

    return result;
}

QString MainWindow::get_style_list()
{
    QMutexLocker    locker(&mainMutex);
    QString         result;
    QTextStream     stream(&result);

    if (scriptMgr.size() > 0) {
        for (const auto & style : styleVec) {
            stream <<  style->GetLabel() << ":" <<
                       style->GetHeadingFontSpec() << ":" <<
                       style->GetMessageFontSpec() << ":" <<
                       style->GetBgColor().name() << ":" <<
                       style->GetFgColor().name() << endl;
        }
        stream << "OK";
    } else {
        stream << "OK";
    }

    return result;
}

void MainWindow::enableDateTime(bool enabled) {
    QMutexLocker    locker(&mainMutex);
    bTimeEnabled = enabled;
}

void MainWindow::setHeading(const QString & heading) {
    QMutexLocker    locker(&mainMutex);
    qInfo() << "Set Heading : " << heading;
    sHeading = heading;
    repaint();
}

void MainWindow::setMessage(const QString & message) {
    QMutexLocker    locker(&mainMutex);
    qInfo() << "Set Message : " << message;
    sMessage = message;
    repaint();
}

/**
 * On the timer event grab the current date/time and refresh the display.
 */
void MainWindow::onTimer() {
#ifdef DEBUG_TIMER
    qDebug() << Q_FUNC_INFO;
#endif
    sDateTime = QDateTime::currentDateTime().toString();
    repaint();
}

void MainWindow::onConnection()
{
    QTcpSocket * clientConnection = tcpServer->nextPendingConnection();

    connect(clientConnection, &QAbstractSocket::disconnected,
            clientConnection, &QObject::deleteLater);

    connect(clientConnection, &QAbstractSocket::readyRead,
            this, [=]() {
                onReadReady(clientConnection);
            });
}

void MainWindow::onReadReady(QTcpSocket * clientConnection)
{
#ifdef DEBUG
    qDebug() << Q_FUNC_INFO;
#endif

    bool bDisconnect = false;

    while (!bDisconnect && clientConnection->canReadLine()) {
        QByteArray data = clientConnection->readLine();
        QString response;
        QString line = QString(data).replace("\n", "").replace("\r", "");
#ifdef DEBUG
        qDebug() << line;
#endif

        bDisconnect = parseText(line, response);
        response += "\n";

        clientConnection->write(response.toUtf8());

        if (bDisconnect) {
            clientConnection->disconnectFromHost();
            clientConnection = nullptr;
        }
    }
}

void MainWindow::processComplete(int exitCode, QProcess::ExitStatus exitStatus)
{
#ifdef DEBUG
    qDebug() << Q_FUNC_INFO;
#endif

    if (exitStatus == QProcess::NormalExit) {
        qInfo() << "Background process exited normally with code " << exitCode;
    } else {
        qInfo() << "Background Process crashed with code " << exitCode;
    }

    bgProcess.close();
    bgRunning = false;

    emit logMessage("STOP", runningScriptName, exitCode);

    QString logFileName = scriptLogFile->fileName();

    scriptLogFile->close();
    delete scriptLogFile;
    scriptLogFile = nullptr;

    QFile resultFile(logFileName);
    QString log;
    if (resultFile.open(QIODevice::ReadOnly)) {
        log = resultFile.readAll();
    }
    if (!log.isEmpty()) {
        emit logMessage("LOG", log, 0);
    }

    resultFile.remove();

    runningScriptName.clear();
}

void MainWindow::runCommand(QString command)
{
    startBgProcess(command);
    return;
}

void MainWindow::stopCommand()
{
    stopBgProcess();
    return;
}

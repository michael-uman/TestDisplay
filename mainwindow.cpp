#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QTcpSocket>
#include <QDate>
#include <QCoreApplication>
#include "mainwindow.h"
#include "displayxmlparser.h"

#define STYLE_XML_PATH      "/opt/TestDisplay/bin/styles.xml"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    if (!StartServer()) {
        qDebug() << "Exiting application...";
        close();
        return;
    }

#ifdef DEBUG
    QSize WinSize = size();
    qDebug() << "Window Size = " << WinSize;
#endif

    setAttribute(Qt::WA_AcceptTouchEvents);
    QMainWindow::showFullScreen();

    // Read the styles from XML
    DisplayXmlParser parser(styleVec);
    QXmlSimpleReader reader;

    reader.setContentHandler(&parser);
    reader.setErrorHandler(&parser);

    QFile file(STYLE_XML_PATH);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "Unable to open file!";
    } else {
        QXmlInputSource xmlSource(&file);
        if (!reader.parse(xmlSource)) {
            qDebug() << "Error parsing the XML file";
        }
    }

    // If there are no styles in the vector, create a default style for use in mean time...
    if (styleVec.length() > 0) {
        style = styleVec[0];
    } else {
        DisplayStylePtr defaultStyle(new DisplayStyle("Default", QColor("yellow"), QColor("blue")));
        styleVec.push_back(defaultStyle);
        style = defaultStyle;
    }

    sDateTime = QDateTime::currentDateTime().toString();

    bRunning = true;

    repaint();

    updateTimer.setInterval(250);
    connect(&updateTimer, &QTimer::timeout, this, &MainWindow::onTimer);
    updateTimer.start();
}

MainWindow::~MainWindow()
{
//    delete ui;
}

QSize MainWindow::getTextSize(QString text, QPainter & p) {
    QFontMetrics fm = p.fontMetrics();
    int tw = fm.width(text);
    int th = fm.height();
    return QSize(tw, th);
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
        p.drawText(64, sh - 24, "Hit the 'Q' key to EXIT");

        sText = "© 2019 Wunder-Bar, Inc.";
        txtSize = getTextSize(sText, p);

        p.drawText(sw - 64 - txtSize.width(), sh - 24, sText);
    }

    QWidget::paintEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent * event)
{
#ifdef DEBUG
    qDebug() << Q_FUNC_INFO;
#endif

    if (event->key() == Qt::Key_Q) {
        QCoreApplication::quit();
    }
}

void MainWindow::closeEvent(QCloseEvent *)
{
#ifdef DEBUG
    qDebug() << Q_FUNC_INFO;
#endif
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

    sMessage = "Touched";
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
    sHeading = DEFAULT_HEADING;
    sMessage = DEFAULT_MESSAGE;
    repaint();
}

/**
 *  Set the text in the display parsing the tag for header/message.
 */
bool MainWindow::setText(QString line)
{
    bool bResult = false;
    QStringList splitted = line.split(':');
    int splitsize = splitted.length();
#ifdef DEBUG
    qDebug() << splitted;
#endif
    if (splitsize == 1) {
        setMessage(splitted[0]);
    } else if (splitsize == 2) {
        if (splitted[0] == "HEAD") {
            setHeading(splitted[1]);
        } else if (splitted[0] == "MESG") {
            setMessage(splitted[1]);
        } else if (splitted[0] == "QUIT") {
            bResult = true;
        } else if (splitted[0] == "TIME") {
            bTimeEnabled = (splitted[1] == "1");
        } else if (splitted[0] == "REST") {
            resetText();
        } else if (splitted[0] == "STYL") {
            int index = splitted[1].toInt();
            if (index < styleVec.length()) {
                style = styleVec[index];
            }
        } else {
            qDebug() << "Unknown tag " << splitted[0];
        }
    } else if (splitsize == 3) {
        if (splitted[0] == "TEXT") {
            setHeading(splitted[1]);
            setMessage(splitted[2]);
        }
    }

    return bResult;
}

void MainWindow::enableDateTime(bool enabled) {
    bTimeEnabled = enabled;
}

void MainWindow::setHeading(const QString & heading) {
    sHeading = heading;
    repaint();
}

void MainWindow::setMessage(const QString & message) {
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
        QString line = QString(data).replace("\n", "").replace("\r", "");
#ifdef DEBUG
        qDebug() << line;
#endif

        bDisconnect = setText(line);

        clientConnection->write("OK\n", 3);

        if (bDisconnect) {
            clientConnection->disconnectFromHost();
            clientConnection = nullptr;
        }
    }
}

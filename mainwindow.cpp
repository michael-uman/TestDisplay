#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QTcpSocket>
#include <QDate>
#include <QCoreApplication>
#include "mainwindow.h"
//#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
//    , ui(new Ui::MainWindow)
{
    if (!StartServer()) {
        qDebug() << "Exiting application...";
        close();
        return;
    }

//    ui->setupUi(this);
    QMainWindow::showFullScreen();
    repaint();

    updateTimer.setInterval(500);

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

void MainWindow::paintEvent(QPaintEvent *)
{
    QString sText;
    QSize size = this->size();
    int sw = size.width();
    int sh = size.height();
    QPainter p(this);
    QColor bgColor(40, 26, 13);
    QBrush bgBrush(bgColor);
    QSize txtSize;

    // Fill the window with the background color
    p.fillRect(0, 0, sw, sh, bgBrush);

    // Draw the text in white
    p.setPen(Qt::white);


    p.setFont(QFont(heading_font_name, heading_font_size));

    txtSize = getTextSize(sHeading, p);

    p.drawText((sw - txtSize.width())/2, 96, sHeading);

    p.setFont(QFont(message_font_name, message_font_size));
    txtSize = getTextSize(sMessage, p);
    // Draw the message in the center of the window
    p.drawText((sw - txtSize.width())/2, (sh - txtSize.height())/2, sMessage);

    // Draw date/time

    p.setFont(QFont("Ubuntu Light", 22));
    txtSize = getTextSize(sDateTime, p);
    p.drawText((sw - txtSize.width())/2, sh - 130, sDateTime);

    p.setFont(QFont("Bitstream Vera Sans Mono", 12));
    p.drawText(64, sh - 24, "Hit the 'Q' key to EXIT");

    sText = "© 2019 Wunder-Bar, Inc.";
    txtSize = getTextSize(sText, p);

    p.drawText(sw - 64 - txtSize.width(), sh - 24, sText);
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
    clientConnection    = nullptr;
}

/**
 *  Set the text in the display parsing the tag for header/message.
 */
void MainWindow::setText(QString line)
{
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
        } else {
            qDebug() << "Unknown tag " << splitted[0];
        }
    }
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
#ifdef DEBUG
    qDebug() << Q_FUNC_INFO;
#endif
    sDateTime = QDateTime::currentDateTime().toString();
    repaint();
}

void MainWindow::onConnection()
{
    clientConnection = tcpServer->nextPendingConnection();

    connect(clientConnection, &QAbstractSocket::disconnected,
            clientConnection, &QObject::deleteLater);

    connect(clientConnection, &QAbstractSocket::readyRead,
            this, &MainWindow::onReadReady);
}

void MainWindow::onReadReady()
{
#ifdef DEBUG
    qDebug() << Q_FUNC_INFO;
#endif

    QByteArray data = clientConnection->readLine();
    QString line = QString(data).replace("\n", "");
#ifdef DEBUG
    qDebug() << line;
#endif
    setText(line);

    clientConnection->disconnectFromHost();
    clientConnection = nullptr;
}

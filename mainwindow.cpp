#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QTcpSocket>
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

void MainWindow::paintEvent(QPaintEvent *)
{
    QSize size = this->size();
    int sw = size.width();
    int sh = size.height();
    QPainter p(this);
    QColor bgColor(40, 26, 13);
    QBrush bgBrush(bgColor);

    // Fill the window with the background color
    p.fillRect(0, 0, sw, sh, bgBrush);

    // Draw the text in white
    p.setPen(Qt::white);


    p.setFont(QFont(heading_font_name, heading_font_size));

    QFontMetrics fm = p.fontMetrics();
    int tw = fm.width(sHeading);
    int th;

    p.drawText((sw - tw)/2, 96, sHeading);

    p.setFont(QFont(message_font_name, message_font_size));
    fm = p.fontMetrics();
    tw = fm.width(sMessage);
    th = fm.height();

    // Draw the message in the center of the window
    p.drawText((sw - tw)/2, (sh - th)/2, sMessage);

    p.setFont(QFont("courier bold", 12));
    p.drawText(64, sh - 24, "Hit the 'Q' key to EXIT");
}

void MainWindow::keyPressEvent(QKeyEvent * event)
{
#ifdef DEBUG
    qDebug() << Q_FUNC_INFO;
#endif

    if (event->key() == Qt::Key_Q) {
        close();
    }
}

void MainWindow::closeEvent(QCloseEvent *)
{
#ifdef DEBUG
    qDebug() << Q_FUNC_INFO;
#endif
    StopServer();
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

void MainWindow::onTimer() {
#ifdef DEBUG
    qDebug() << Q_FUNC_INFO;
#endif
    repaint();
}

void MainWindow::onConnection()
{
    clientConnection = tcpServer->nextPendingConnection();

    connect(clientConnection, &QAbstractSocket::disconnected,
            clientConnection, &QObject::deleteLater);

    connect(clientConnection, &QAbstractSocket::readyRead, this, &MainWindow::onReadReady);
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

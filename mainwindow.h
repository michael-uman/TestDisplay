#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QTimer>
#include <QTcpServer>



#define HEADING_FONT_NAME       "Helvetica"
#define HEADING_FONT_POINTS     32
#define MESSAGE_FONT_NAME       "Verdana"
#define MESSAGE_FONT_POINTS     24

#define SERVER_PORT             4321

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void setHeading(const QString & heading);
    void setMessage(const QString & message);


public slots:
    void onTimer();
    void onConnection();
    void onReadReady();

protected:
    void paintEvent(QPaintEvent * event) override;
    void keyPressEvent(QKeyEvent * event) override;
    void closeEvent(QCloseEvent * event) override;

    bool StartServer();
    void StopServer();

    void setText(QString line);


private:
    QTimer          updateTimer;
    QTcpServer *    tcpServer           = nullptr;
    QTcpSocket *    clientConnection    = nullptr;

    QString         sHeading = "Test Display Application";
    QString         sMessage = "(Waiting for client connection)";

    int             server_port         = SERVER_PORT;
    QString         heading_font_name   = HEADING_FONT_NAME;
    int             heading_font_size   = HEADING_FONT_POINTS;
    QString         message_font_name   = MESSAGE_FONT_NAME;
    int             message_font_size   = MESSAGE_FONT_POINTS;
};

#endif // MAINWINDOW_H

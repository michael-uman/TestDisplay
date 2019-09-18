#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QTimer>
#include <QTcpServer>
#include <QTouchDevice>
#include <QTouchEvent>
#include <QProcess>

#include "testdisplay_defaults.h"
#include "displaystyle.h"
#include "testscriptmanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void setHeading(const QString & heading);
    void setMessage(const QString & message);

    void enableDateTime(bool enabled);

    inline bool inUse() const {
        return bDisplayInUse;
    }

public slots:
    void onTimer();
    void onConnection();
    void onReadReady(QTcpSocket * clientSocket);
    void processComplete(int exitCode, QProcess::ExitStatus exitStatus);

protected:
    void paintEvent(QPaintEvent * event) override;
    void keyPressEvent(QKeyEvent * event) override;
    void closeEvent(QCloseEvent * event) override;
    void resizeEvent(QResizeEvent * event) override;
    bool event(QEvent * event) override;
    void mytouchEvent(QTouchEvent * event);

    bool            StartServer();
    void            StopServer();

    bool            setText(QString line, QString & response);
    QSize           getTextSize(QString text, QPainter & p);
    void            resetText();

    bool            startBgProcess(QString script_path);
    bool            loadStyles();
    bool            loadScripts();

private:
    bool            bRunning            = false;
    QTimer          updateTimer;
    QTcpServer *    tcpServer           = nullptr;
    bool            bDisplayInUse       = false;

    QString         sHeading            = DEFAULT_HEADING;
    QString         sMessage            = DEFAULT_MESSAGE;

    bool            bTimeEnabled        = true;
    QString         sDateTime;

    quint16         server_port         = SERVER_PORT;

    DisplayStylePtr style;
    DisplayStyleVec styleVec;

    bool            bgRunning           = false;
    QProcess        bgProcess;

    qint64          appPid              = -1;

    TestScriptMgr   scriptMgr;
};

#endif // MAINWINDOW_H

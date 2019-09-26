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
#include "scheduler.h"

enum class displayState {
    DISPLAY_BLANK,
    DISPLAY_MAIN,
    DISPLAY_MENU,
    DISPLAY_DIALOG,
    DISPLAY_TEST,
};

enum class optionFile {
    OPTION_SETTINGS,
    OPTION_STYLES,
    OPTION_SCRIPTS,
    OPTION_SCHEDULE,
};

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
    void runCommand(QString command);

protected:
    void paintEvent(QPaintEvent * event) override;
    void keyPressEvent(QKeyEvent * event) override;
    void closeEvent(QCloseEvent * event) override;
    void resizeEvent(QResizeEvent * event) override;
    bool event(QEvent * event) override;
    void mytouchEvent(QTouchEvent * event);

    void            paintMainDisplay();
    void            paintMenuDisplay();

    bool            StartServer();
    void            StopServer();

    bool            parseText(QString line, QString & response);
    QSize           getTextSize(QString text, QPainter & p);
    void            resetText();

    bool            startBgProcess(QString script_path);
    bool            stopBgProcess();

    QString         getOptionPath(enum optionFile opt);

    bool            loadStyles();
    bool            loadScripts();
    QString         get_status_string();
    QString         get_script_list();
    QString         get_style_list();

private:
    bool            bRunning            = false;
    displayState    state               = displayState::DISPLAY_BLANK;

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
    QString         runningScriptName;
    QProcess        bgProcess;

    qint64          appPid              = -1;

    TestScriptMgr   scriptMgr;

    Scheduler       sched;

    static QStringList     optionFilenames;
};

#endif // MAINWINDOW_H

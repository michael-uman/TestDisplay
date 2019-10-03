#include <QCoreApplication>
#include <QObject>
#include <QDebug>
#include <QMutex>
#include <QDir>
#include "mainwindow.h"
#include "testdisplayrequesthandler.h"
#include "testscriptmanager.h"

TestDisplayRequestHandler::TestDisplayRequestHandler(QObject *parent)
    : HttpRequestHandler(parent)
{
    MainWindow * pParent = static_cast<MainWindow*>(parent);

    qDebug() << Q_FUNC_INFO;

    connect(this, &TestDisplayRequestHandler::runCommand, pParent, &MainWindow::runCommand);
    connect(this, &TestDisplayRequestHandler::stopCommand, pParent, &MainWindow::stopCommand);
}

TestDisplayRequestHandler::~TestDisplayRequestHandler()
{
    qDebug() << Q_FUNC_INFO;
}

QString TestDisplayRequestHandler::getHead() {
    QString sResponseText;

    sResponseText += "<head>\n"
                     "\t<title>Test Display</title>\n"
                     "\t<link rel=\"stylesheet\" href=\"/styles/testdisplay.css\"\n"
                     "</head>\n";

    return sResponseText;
}

QString TestDisplayRequestHandler::getToolbar() {
    MainWindow *    pParent = static_cast<MainWindow*>(parent());
    QString         sToolbar;

    sToolbar =  "<ul class=\"toolbar\">\n";
    if (pParent->bgRunning) {
        sToolbar += "  <li class=\"toolbar\"><a href=\"/stopscript\">Stop Script</a></li>\n";
    } else {
        sToolbar += "  <li class=\"toolbar\"><a href=\"/runscript\">Start Script</a></li>\n";
    }
    sToolbar += "  <li class=\"toolbar\"><a href=\"/stat/\">Display Status</a></li>\n";
    sToolbar += "</ul>\n";
    return sToolbar;
}

void TestDisplayRequestHandler::service(HttpRequest &request, HttpResponse &response)
{
    MainWindow *    pParent = static_cast<MainWindow*>(parent());
    QMutexLocker    locker(&pParent->mainMutex);
    QString         reqPath = request.getPath();
    QString         sResponseText;

//    qDebug() << "User requested URL " + reqPath;

    response.setHeader("Content-Type", "text/html; charset=ISO-8859-1");

    if (reqPath == "/") {
        response.setHeader("Refresh", "5;url=/");

        sResponseText = "<html>";
        sResponseText += getHead();
        sResponseText += "<body>";
        sResponseText += "<center><h1>Test Display Web Interface</h1></center>";
        sResponseText += getToolbar();
        sResponseText += "</body></html>";
        response.write(sResponseText.toUtf8());
    } else if (reqPath.startsWith("/runscript")) {
        sResponseText = "<html>";
        sResponseText += getHead();
        sResponseText += "<body>";
        sResponseText += "<center><h1>Test Display Web Interface</h1></center>";
        sResponseText += getToolbar();

        sResponseText += "<h2>List of Available Scripts</h2><ul>";
        for (const auto & script : pParent->scriptMgr.getVec()) {
            QString hRef = "run/" + script->key();
            sResponseText += "<li><a href=\"" + hRef + "\">" + script->name() + "</a></li>";
        }

        response.write(sResponseText.toUtf8());
//        sResponseText += "</ul><br>";
//        if (pParent->bgRunning) {
//            sResponseText += "<p><a href=\"xxxyyyzzz\">Stop Running Process</a></p>";
//        }
//        sResponseText += "<p><a href=\"/stat/\">Display Status</a></p>";
    } else if (reqPath.startsWith("/run/")) {
        QString scriptKey = reqPath.remove(0, 5);
        TestScriptPtr scriptInfo = pParent->scriptMgr.getScriptForKey(scriptKey.front().toLatin1());

        emit runCommand(scriptInfo->path());

        sResponseText = "<html>";
        sResponseText += getHead(); // <head><title>Test Display</title></head>
        sResponseText += "<body>";
        sResponseText += "<center><h1>Test Display Web Interface</h1></center>";
        sResponseText += getToolbar();
        sResponseText += "<p><b>Script " + scriptInfo->name() + " (" + scriptInfo->path() + ") is running</b></p>";
//        sResponseText += "<br>";
//        sResponseText += "<a href=\"/\">Go back to home page</a>";
        sResponseText += "</body></html>";

        response.write(sResponseText.toUtf8());
    } else if (reqPath.startsWith("/stat/")) {
        response.setHeader("Refresh", "2;url=/stat/");
        sResponseText = "<html>";
        sResponseText += getHead();
        sResponseText += "<body>";
        sResponseText += "<center><h1>Test Display Web Interface</h1></center>";
        sResponseText += getToolbar();
        sResponseText += "<h3>Current status</h3>";
        sResponseText += "<table border=\"1\" style=\"font-family:monospace; font-size: 12pt;\">";
        sResponseText += "<tr><td>Heading Text</td><td>" + pParent->sHeading + "</td></tr>";
        sResponseText += "<tr><td>Message Text</td><td>" + pParent->sMessage + "</td></tr>";
        sResponseText += "<tr><td>Time Display</td><td>" + QString(pParent->bTimeEnabled?"Enabled":"Disabled") + "</td></tr>";
        sResponseText += "<tr><td>Display Style</td><td>" + pParent->style->GetLabel() + "</td></tr>";
        sResponseText += "<tr><td>Running Script</td><td>" + (pParent->bgRunning?pParent->runningScriptName:QString("NONE")) + "</td></tr>";
        sResponseText += "</table>";
        sResponseText += "<br>";
        sResponseText += "<h3>Available Scripts</h3>";
        sResponseText += "<table border=\"1\" style=\"font-family:monospace; font-size: 12pt;\">";
        sResponseText += "<tr><th>Key</th><th>Name</th><th>Path</th></tr>";
        for (const auto & script : pParent->scriptMgr.getVec()) {
            sResponseText += "<tr><td>" + script->key() + "</td><td>" + script->name() + "</td><td>" + script->path() + "</td></tr>";
        }
        sResponseText += "</table>";
        sResponseText += "</body></html>";

        response.write(sResponseText.toUtf8());
    } else if (reqPath.startsWith("/stopscript")) {
        response.setHeader("Refresh", "2;url=/");
        sResponseText = "<html>";
        sResponseText += getHead();
        sResponseText += "<body>";
        sResponseText += "<h1>Stopping Process</h1>";
        sResponseText += "</body></html>";
        response.write(sResponseText.toUtf8());
        emit stopCommand();
    } else if (reqPath.startsWith("/images/")) {
        QString appDir = QCoreApplication::applicationDirPath();
        QString imageName = reqPath.remove(0, 8);
        response.write(QString("Image " + appDir + "/" + imageName).toLatin1());
    } else if (reqPath.startsWith("/styles/")) {
        QString styleName = reqPath.remove(0, 8);
        QString basePath = "/opt/TestDisplay/webinterface/styles/" + styleName;
        if (QFileInfo::exists(basePath)) {
            QFile file(basePath);

            if (file.open(QIODevice::ReadOnly)) {
                QString cssText = file.readAll();
                response.write(cssText.toUtf8());
            } else {
                response.setStatus(404);
            }
        }
//        response.write(QString("Style " + styleName).toLatin1());
    }else {
        response.setStatus(404);
    }

}

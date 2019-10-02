#ifndef TESTDISPLAYREQUESTHANDLER_H
#define TESTDISPLAYREQUESTHANDLER_H

#include <QObject>
#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

using HttpRequest        = stefanfrings::HttpRequest;
using HttpResponse       = stefanfrings::HttpResponse;
using HttpRequestHandler = stefanfrings::HttpRequestHandler;

class TestDisplayRequestHandler : public HttpRequestHandler {
    Q_OBJECT
    Q_DISABLE_COPY(TestDisplayRequestHandler)


signals:
    void runCommand(QString command);
    void stopCommand();

public:
    TestDisplayRequestHandler(QObject * parent = nullptr);
    ~TestDisplayRequestHandler();

    void service(HttpRequest & request, HttpResponse & response);
};

#endif // TESTDISPLAYREQUESTHANDLER_H

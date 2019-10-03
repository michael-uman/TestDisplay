QT       += core gui network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG(debug, debug|release) {
    DEFINES += DEBUG
}

DEFINES += "VERSION_MAJOR=1"
DEFINES += "VERSION_MINOR=1"
DEFINES += "VERSION_BUILD=2"

SOURCES += \
    displaystyle.cpp \
    displayxmlparser.cpp \
    main.cpp \
    mainwindow.cpp \
    rpi_utils.cpp \
    runguard.cpp \
    scheduler.cpp \
    testdisplayrequesthandler.cpp \
    testscriptmanager.cpp

HEADERS += \
    displaystyle.h \
    displayxmlparser.h \
    mainwindow.h \
    processmanager.h \
    rpi_utils.h \
    runguard.h \
    scheduler.h \
    testdisplay_defaults.h \
    testdisplayrequesthandler.h \
    testscriptmanager.h

unix {
    SOURCES += linuxprocessentry.cpp \
               linuxprocessmanager.cpp
    HEADERS += linuxprocessentry.h \
               linuxprocessmanager.h
}

win32 {
    SOURCES += windowsprocessentry.cpp \
               windowsprocessmanager.cpp

    HEADERS += windowsprocessentry.h \
               windowsprocessmanager.h

    LIBS += -lpsapi
}

FORMS +=

xmlfiles.path = /opt/$${TARGET}/bin
xmlfiles.files += xml/styles.xml

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target xmlfiles

DISTFILES += \
    xml/schedule.xml \
    xml/scripts.xml \
    xml/styles.xml

# include(../QtWebApp/QtWebApp/logging/logging.pri)
include(../QtWebApp/QtWebApp/httpserver/httpserver.pri)
include(../QtWebApp/QtWebApp/templateengine/templateengine.pri)


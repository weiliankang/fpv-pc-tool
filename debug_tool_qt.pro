QT       += core gui widgets serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = debug_tool_qt
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    serialprotocolhandler.cpp \
    serialcommunicator.cpp \
    firmwareparser.cpp \
    translator.cpp \
    osdgridwidget.cpp

HEADERS += \
    mainwindow.h \
    serialprotocolhandler.h \
    serialcommunicator.h \
    firmwareparser.h \
    translator.h \
    osdgridwidget.h

FORMS += \
    mainwindow.ui \
    page_firmware.ui \
    page_serial_connection.ui \
    page_serial_keycontrol.ui \
    page_serial_wireless.ui \
    page_serial_osd.ui \
    page_serial_custom.ui \
    page_serial_history.ui \
    page_settings.ui

RESOURCES += osdchars.qrc \
    osdchars720.qrc

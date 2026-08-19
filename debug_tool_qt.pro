QT       += core gui widgets serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
TARGET = debug_tool_qt
TEMPLATE = app
RC_ICONS = appicon.ico

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    serialprotocolhandler.cpp \
    serialcommunicator.cpp \
    firmwareparser.cpp \
    translator.cpp \
    osdgridwidget.cpp \
    filltextedit.cpp \
    fillplaintextedit.cpp \
    crsftestpage.cpp \
    crsfprotocolparser.cpp \
    crsfchannelwidget.cpp \
    sbustestpage.cpp \
    sbusprotocolparser.cpp \
    mavlinktestpage.cpp \
    mavlinkprotocolparser.cpp \
    f0readpage.cpp

HEADERS += \
    mainwindow.h \
    serialprotocolhandler.h \
    serialcommunicator.h \
    firmwareparser.h \
    translator.h \
    osdgridwidget.h \
    filltextedit.h \
    fillplaintextedit.h \
    crsftestpage.h \
    crsfprotocolparser.h \
    crsfchannelwidget.h \
    sbustestpage.h \
    sbusprotocolparser.h \
    mavlinktestpage.h \
    mavlinkprotocolparser.h \
    f0readpage.h

FORMS += \
    mainwindow.ui \
    page_firmware.ui \
    page_serial_connection.ui \
    page_serial_keycontrol.ui \
    page_serial_wireless.ui \
    page_serial_osd.ui \
    page_serial_custom.ui \
    page_serial_history.ui \
    page_serial_crsftest.ui \
    page_serial_sbustest.ui \
    page_serial_mavlinktest.ui \
    page_serial_f0read.ui \
    page_settings.ui

RESOURCES += osdchars.qrc \
    osdchars720.qrc


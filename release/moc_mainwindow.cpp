/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[56];
    char stringdata0[782];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 16), // "onSidebarChanged"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 5), // "index"
QT_MOC_LITERAL(4, 35, 21), // "onSerialStatusChanged"
QT_MOC_LITERAL(5, 57, 3), // "msg"
QT_MOC_LITERAL(6, 61, 11), // "onSkyBrowse"
QT_MOC_LITERAL(7, 73, 14), // "onGroundBrowse"
QT_MOC_LITERAL(8, 88, 10), // "onSkyParse"
QT_MOC_LITERAL(9, 99, 13), // "onGroundParse"
QT_MOC_LITERAL(10, 113, 14), // "onRefreshPorts"
QT_MOC_LITERAL(11, 128, 18), // "onToggleConnection"
QT_MOC_LITERAL(12, 147, 12), // "onKeyCommand"
QT_MOC_LITERAL(13, 160, 4), // "name"
QT_MOC_LITERAL(14, 165, 15), // "onSendCustomKey"
QT_MOC_LITERAL(15, 181, 18), // "onUpdateKeyPreview"
QT_MOC_LITERAL(16, 200, 15), // "setupKeyButtons"
QT_MOC_LITERAL(17, 216, 9), // "onSetFreq"
QT_MOC_LITERAL(18, 226, 9), // "onGetFreq"
QT_MOC_LITERAL(19, 236, 11), // "onGetStatus"
QT_MOC_LITERAL(20, 248, 10), // "onGetBbPwr"
QT_MOC_LITERAL(21, 259, 10), // "onSetBbPwr"
QT_MOC_LITERAL(22, 270, 14), // "onGetStatusSky"
QT_MOC_LITERAL(23, 285, 13), // "onGetDistance"
QT_MOC_LITERAL(24, 299, 18), // "previewWirelessCmd"
QT_MOC_LITERAL(25, 318, 20), // "wireless_data_type_t"
QT_MOC_LITERAL(26, 339, 4), // "type"
QT_MOC_LITERAL(27, 344, 6), // "values"
QT_MOC_LITERAL(28, 351, 25), // "onUpdatePreviewFromParams"
QT_MOC_LITERAL(29, 377, 14), // "onSetRelayFreq"
QT_MOC_LITERAL(30, 392, 14), // "onGetRelayFreq"
QT_MOC_LITERAL(31, 407, 16), // "onGetRelayStatus"
QT_MOC_LITERAL(32, 424, 15), // "onGetRelayBbPwr"
QT_MOC_LITERAL(33, 440, 15), // "onSetRelayBbPwr"
QT_MOC_LITERAL(34, 456, 17), // "onGetRelayGndDist"
QT_MOC_LITERAL(35, 474, 17), // "onGetRelaySkyDist"
QT_MOC_LITERAL(36, 492, 20), // "onGetOsdDataWireless"
QT_MOC_LITERAL(37, 513, 22), // "onClearOsdDataWireless"
QT_MOC_LITERAL(38, 536, 12), // "onGetOsdData"
QT_MOC_LITERAL(39, 549, 10), // "onClearOsd"
QT_MOC_LITERAL(40, 560, 15), // "onFcTypeChanged"
QT_MOC_LITERAL(41, 576, 17), // "onAutoPollToggled"
QT_MOC_LITERAL(42, 594, 7), // "checked"
QT_MOC_LITERAL(43, 602, 22), // "onOsdResolutionChanged"
QT_MOC_LITERAL(44, 625, 11), // "onPollTimer"
QT_MOC_LITERAL(45, 637, 17), // "onOsdDataReceived"
QT_MOC_LITERAL(46, 655, 11), // "dataContent"
QT_MOC_LITERAL(47, 667, 13), // "onCheckFormat"
QT_MOC_LITERAL(48, 681, 12), // "onSendCustom"
QT_MOC_LITERAL(49, 694, 11), // "onClearSend"
QT_MOC_LITERAL(50, 706, 11), // "onClearRecv"
QT_MOC_LITERAL(51, 718, 12), // "onExportSend"
QT_MOC_LITERAL(52, 731, 12), // "onExportRecv"
QT_MOC_LITERAL(53, 744, 14), // "onDataReceived"
QT_MOC_LITERAL(54, 759, 4), // "data"
QT_MOC_LITERAL(55, 764, 17) // "onLanguageChanged"

    },
    "MainWindow\0onSidebarChanged\0\0index\0"
    "onSerialStatusChanged\0msg\0onSkyBrowse\0"
    "onGroundBrowse\0onSkyParse\0onGroundParse\0"
    "onRefreshPorts\0onToggleConnection\0"
    "onKeyCommand\0name\0onSendCustomKey\0"
    "onUpdateKeyPreview\0setupKeyButtons\0"
    "onSetFreq\0onGetFreq\0onGetStatus\0"
    "onGetBbPwr\0onSetBbPwr\0onGetStatusSky\0"
    "onGetDistance\0previewWirelessCmd\0"
    "wireless_data_type_t\0type\0values\0"
    "onUpdatePreviewFromParams\0onSetRelayFreq\0"
    "onGetRelayFreq\0onGetRelayStatus\0"
    "onGetRelayBbPwr\0onSetRelayBbPwr\0"
    "onGetRelayGndDist\0onGetRelaySkyDist\0"
    "onGetOsdDataWireless\0onClearOsdDataWireless\0"
    "onGetOsdData\0onClearOsd\0onFcTypeChanged\0"
    "onAutoPollToggled\0checked\0"
    "onOsdResolutionChanged\0onPollTimer\0"
    "onOsdDataReceived\0dataContent\0"
    "onCheckFormat\0onSendCustom\0onClearSend\0"
    "onClearRecv\0onExportSend\0onExportRecv\0"
    "onDataReceived\0data\0onLanguageChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      45,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,  239,    2, 0x08 /* Private */,
       4,    1,  242,    2, 0x08 /* Private */,
       6,    0,  245,    2, 0x08 /* Private */,
       7,    0,  246,    2, 0x08 /* Private */,
       8,    0,  247,    2, 0x08 /* Private */,
       9,    0,  248,    2, 0x08 /* Private */,
      10,    0,  249,    2, 0x08 /* Private */,
      11,    0,  250,    2, 0x08 /* Private */,
      12,    1,  251,    2, 0x08 /* Private */,
      14,    0,  254,    2, 0x08 /* Private */,
      15,    0,  255,    2, 0x08 /* Private */,
      16,    0,  256,    2, 0x08 /* Private */,
      17,    0,  257,    2, 0x08 /* Private */,
      18,    0,  258,    2, 0x08 /* Private */,
      19,    0,  259,    2, 0x08 /* Private */,
      20,    0,  260,    2, 0x08 /* Private */,
      21,    0,  261,    2, 0x08 /* Private */,
      22,    0,  262,    2, 0x08 /* Private */,
      23,    0,  263,    2, 0x08 /* Private */,
      24,    3,  264,    2, 0x08 /* Private */,
      28,    0,  271,    2, 0x08 /* Private */,
      29,    0,  272,    2, 0x08 /* Private */,
      30,    0,  273,    2, 0x08 /* Private */,
      31,    0,  274,    2, 0x08 /* Private */,
      32,    0,  275,    2, 0x08 /* Private */,
      33,    0,  276,    2, 0x08 /* Private */,
      34,    0,  277,    2, 0x08 /* Private */,
      35,    0,  278,    2, 0x08 /* Private */,
      36,    0,  279,    2, 0x08 /* Private */,
      37,    0,  280,    2, 0x08 /* Private */,
      38,    0,  281,    2, 0x08 /* Private */,
      39,    0,  282,    2, 0x08 /* Private */,
      40,    1,  283,    2, 0x08 /* Private */,
      41,    1,  286,    2, 0x08 /* Private */,
      43,    1,  289,    2, 0x08 /* Private */,
      44,    0,  292,    2, 0x08 /* Private */,
      45,    1,  293,    2, 0x08 /* Private */,
      47,    0,  296,    2, 0x08 /* Private */,
      48,    0,  297,    2, 0x08 /* Private */,
      49,    0,  298,    2, 0x08 /* Private */,
      50,    0,  299,    2, 0x08 /* Private */,
      51,    0,  300,    2, 0x08 /* Private */,
      52,    0,  301,    2, 0x08 /* Private */,
      53,    1,  302,    2, 0x08 /* Private */,
      55,    1,  305,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   13,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 25, QMetaType::QByteArray, QMetaType::QString,   26,   27,   13,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Bool,   42,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QByteArray,   46,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QByteArray,   54,
    QMetaType::Void, QMetaType::Int,    3,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onSidebarChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->onSerialStatusChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->onSkyBrowse(); break;
        case 3: _t->onGroundBrowse(); break;
        case 4: _t->onSkyParse(); break;
        case 5: _t->onGroundParse(); break;
        case 6: _t->onRefreshPorts(); break;
        case 7: _t->onToggleConnection(); break;
        case 8: _t->onKeyCommand((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 9: _t->onSendCustomKey(); break;
        case 10: _t->onUpdateKeyPreview(); break;
        case 11: _t->setupKeyButtons(); break;
        case 12: _t->onSetFreq(); break;
        case 13: _t->onGetFreq(); break;
        case 14: _t->onGetStatus(); break;
        case 15: _t->onGetBbPwr(); break;
        case 16: _t->onSetBbPwr(); break;
        case 17: _t->onGetStatusSky(); break;
        case 18: _t->onGetDistance(); break;
        case 19: _t->previewWirelessCmd((*reinterpret_cast< wireless_data_type_t(*)>(_a[1])),(*reinterpret_cast< const QByteArray(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        case 20: _t->onUpdatePreviewFromParams(); break;
        case 21: _t->onSetRelayFreq(); break;
        case 22: _t->onGetRelayFreq(); break;
        case 23: _t->onGetRelayStatus(); break;
        case 24: _t->onGetRelayBbPwr(); break;
        case 25: _t->onSetRelayBbPwr(); break;
        case 26: _t->onGetRelayGndDist(); break;
        case 27: _t->onGetRelaySkyDist(); break;
        case 28: _t->onGetOsdDataWireless(); break;
        case 29: _t->onClearOsdDataWireless(); break;
        case 30: _t->onGetOsdData(); break;
        case 31: _t->onClearOsd(); break;
        case 32: _t->onFcTypeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 33: _t->onAutoPollToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 34: _t->onOsdResolutionChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 35: _t->onPollTimer(); break;
        case 36: _t->onOsdDataReceived((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 37: _t->onCheckFormat(); break;
        case 38: _t->onSendCustom(); break;
        case 39: _t->onClearSend(); break;
        case 40: _t->onClearRecv(); break;
        case 41: _t->onExportSend(); break;
        case 42: _t->onExportRecv(); break;
        case 43: _t->onDataReceived((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 44: _t->onLanguageChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 45)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 45;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 45)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 45;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE

/****************************************************************************
** Meta object code from reading C++ file 'crsftestpage.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../crsftestpage.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'crsftestpage.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CrsfTestPage_t {
    QByteArrayData data[25];
    char stringdata0[329];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CrsfTestPage_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CrsfTestPage_t qt_meta_stringdata_CrsfTestPage = {
    {
QT_MOC_LITERAL(0, 0, 12), // "CrsfTestPage"
QT_MOC_LITERAL(1, 13, 19), // "serialStatusChanged"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 3), // "msg"
QT_MOC_LITERAL(4, 38, 14), // "onRefreshPorts"
QT_MOC_LITERAL(5, 53, 18), // "onToggleConnection"
QT_MOC_LITERAL(6, 72, 10), // "onSendData"
QT_MOC_LITERAL(7, 83, 11), // "onClearRecv"
QT_MOC_LITERAL(8, 95, 10), // "onSaveRecv"
QT_MOC_LITERAL(9, 106, 12), // "onResetStats"
QT_MOC_LITERAL(10, 119, 15), // "onTplRcChannels"
QT_MOC_LITERAL(11, 135, 14), // "onTplLinkStats"
QT_MOC_LITERAL(12, 150, 14), // "onTplHeartbeat"
QT_MOC_LITERAL(13, 165, 11), // "onTplCustom"
QT_MOC_LITERAL(14, 177, 14), // "onDataReceived"
QT_MOC_LITERAL(15, 192, 4), // "data"
QT_MOC_LITERAL(16, 197, 16), // "onRxFlushTimeout"
QT_MOC_LITERAL(17, 214, 14), // "onSerialStatus"
QT_MOC_LITERAL(18, 229, 17), // "onLogFlushTimeout"
QT_MOC_LITERAL(19, 247, 15), // "onStopAtChanged"
QT_MOC_LITERAL(20, 263, 5), // "value"
QT_MOC_LITERAL(21, 269, 14), // "onStopAtToggle"
QT_MOC_LITERAL(22, 284, 7), // "checked"
QT_MOC_LITERAL(23, 292, 15), // "onClearAnalyzer"
QT_MOC_LITERAL(24, 308, 20) // "onResetIntervalStats"

    },
    "CrsfTestPage\0serialStatusChanged\0\0msg\0"
    "onRefreshPorts\0onToggleConnection\0"
    "onSendData\0onClearRecv\0onSaveRecv\0"
    "onResetStats\0onTplRcChannels\0"
    "onTplLinkStats\0onTplHeartbeat\0onTplCustom\0"
    "onDataReceived\0data\0onRxFlushTimeout\0"
    "onSerialStatus\0onLogFlushTimeout\0"
    "onStopAtChanged\0value\0onStopAtToggle\0"
    "checked\0onClearAnalyzer\0onResetIntervalStats"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CrsfTestPage[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      19,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,  109,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    0,  112,    2, 0x08 /* Private */,
       5,    0,  113,    2, 0x08 /* Private */,
       6,    0,  114,    2, 0x08 /* Private */,
       7,    0,  115,    2, 0x08 /* Private */,
       8,    0,  116,    2, 0x08 /* Private */,
       9,    0,  117,    2, 0x08 /* Private */,
      10,    0,  118,    2, 0x08 /* Private */,
      11,    0,  119,    2, 0x08 /* Private */,
      12,    0,  120,    2, 0x08 /* Private */,
      13,    0,  121,    2, 0x08 /* Private */,
      14,    1,  122,    2, 0x08 /* Private */,
      16,    0,  125,    2, 0x08 /* Private */,
      17,    1,  126,    2, 0x08 /* Private */,
      18,    0,  129,    2, 0x08 /* Private */,
      19,    1,  130,    2, 0x08 /* Private */,
      21,    1,  133,    2, 0x08 /* Private */,
      23,    0,  136,    2, 0x08 /* Private */,
      24,    0,  137,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,

 // slots: parameters
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
    QMetaType::Void, QMetaType::QByteArray,   15,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   20,
    QMetaType::Void, QMetaType::Bool,   22,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CrsfTestPage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CrsfTestPage *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->serialStatusChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->onRefreshPorts(); break;
        case 2: _t->onToggleConnection(); break;
        case 3: _t->onSendData(); break;
        case 4: _t->onClearRecv(); break;
        case 5: _t->onSaveRecv(); break;
        case 6: _t->onResetStats(); break;
        case 7: _t->onTplRcChannels(); break;
        case 8: _t->onTplLinkStats(); break;
        case 9: _t->onTplHeartbeat(); break;
        case 10: _t->onTplCustom(); break;
        case 11: _t->onDataReceived((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 12: _t->onRxFlushTimeout(); break;
        case 13: _t->onSerialStatus((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 14: _t->onLogFlushTimeout(); break;
        case 15: _t->onStopAtChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: _t->onStopAtToggle((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 17: _t->onClearAnalyzer(); break;
        case 18: _t->onResetIntervalStats(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CrsfTestPage::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CrsfTestPage::serialStatusChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CrsfTestPage::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CrsfTestPage.data,
    qt_meta_data_CrsfTestPage,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CrsfTestPage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CrsfTestPage::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CrsfTestPage.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CrsfTestPage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 19)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 19;
    }
    return _id;
}

// SIGNAL 0
void CrsfTestPage::serialStatusChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE

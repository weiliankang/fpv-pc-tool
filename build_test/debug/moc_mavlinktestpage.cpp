/****************************************************************************
** Meta object code from reading C++ file 'mavlinktestpage.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../mavlinktestpage.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mavlinktestpage.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MavlinkTestPage_t {
    QByteArrayData data[20];
    char stringdata0[245];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MavlinkTestPage_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MavlinkTestPage_t qt_meta_stringdata_MavlinkTestPage = {
    {
QT_MOC_LITERAL(0, 0, 15), // "MavlinkTestPage"
QT_MOC_LITERAL(1, 16, 19), // "serialStatusChanged"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 3), // "msg"
QT_MOC_LITERAL(4, 41, 14), // "onRefreshPorts"
QT_MOC_LITERAL(5, 56, 18), // "onToggleConnection"
QT_MOC_LITERAL(6, 75, 10), // "onSendData"
QT_MOC_LITERAL(7, 86, 11), // "onClearRecv"
QT_MOC_LITERAL(8, 98, 10), // "onSaveRecv"
QT_MOC_LITERAL(9, 109, 12), // "onResetStats"
QT_MOC_LITERAL(10, 122, 14), // "onTplHeartbeat"
QT_MOC_LITERAL(11, 137, 11), // "onTplCustom"
QT_MOC_LITERAL(12, 149, 14), // "onDataReceived"
QT_MOC_LITERAL(13, 164, 4), // "data"
QT_MOC_LITERAL(14, 169, 14), // "onSerialStatus"
QT_MOC_LITERAL(15, 184, 15), // "onClearAnalyzer"
QT_MOC_LITERAL(16, 200, 15), // "onStopAtChanged"
QT_MOC_LITERAL(17, 216, 5), // "value"
QT_MOC_LITERAL(18, 222, 14), // "onStopAtToggle"
QT_MOC_LITERAL(19, 237, 7) // "checked"

    },
    "MavlinkTestPage\0serialStatusChanged\0"
    "\0msg\0onRefreshPorts\0onToggleConnection\0"
    "onSendData\0onClearRecv\0onSaveRecv\0"
    "onResetStats\0onTplHeartbeat\0onTplCustom\0"
    "onDataReceived\0data\0onSerialStatus\0"
    "onClearAnalyzer\0onStopAtChanged\0value\0"
    "onStopAtToggle\0checked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MavlinkTestPage[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   84,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    0,   87,    2, 0x08 /* Private */,
       5,    0,   88,    2, 0x08 /* Private */,
       6,    0,   89,    2, 0x08 /* Private */,
       7,    0,   90,    2, 0x08 /* Private */,
       8,    0,   91,    2, 0x08 /* Private */,
       9,    0,   92,    2, 0x08 /* Private */,
      10,    0,   93,    2, 0x08 /* Private */,
      11,    0,   94,    2, 0x08 /* Private */,
      12,    1,   95,    2, 0x08 /* Private */,
      14,    1,   98,    2, 0x08 /* Private */,
      15,    0,  101,    2, 0x08 /* Private */,
      16,    1,  102,    2, 0x08 /* Private */,
      18,    1,  105,    2, 0x08 /* Private */,

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
    QMetaType::Void, QMetaType::QByteArray,   13,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   17,
    QMetaType::Void, QMetaType::Bool,   19,

       0        // eod
};

void MavlinkTestPage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MavlinkTestPage *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->serialStatusChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->onRefreshPorts(); break;
        case 2: _t->onToggleConnection(); break;
        case 3: _t->onSendData(); break;
        case 4: _t->onClearRecv(); break;
        case 5: _t->onSaveRecv(); break;
        case 6: _t->onResetStats(); break;
        case 7: _t->onTplHeartbeat(); break;
        case 8: _t->onTplCustom(); break;
        case 9: _t->onDataReceived((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 10: _t->onSerialStatus((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 11: _t->onClearAnalyzer(); break;
        case 12: _t->onStopAtChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->onStopAtToggle((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MavlinkTestPage::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MavlinkTestPage::serialStatusChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MavlinkTestPage::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_MavlinkTestPage.data,
    qt_meta_data_MavlinkTestPage,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MavlinkTestPage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MavlinkTestPage::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MavlinkTestPage.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int MavlinkTestPage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 14;
    }
    return _id;
}

// SIGNAL 0
void MavlinkTestPage::serialStatusChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE

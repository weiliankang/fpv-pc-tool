/****************************************************************************
** Meta object code from reading C++ file 'f0readpage.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../f0readpage.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'f0readpage.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_F0ReadPage_t {
    QByteArrayData data[18];
    char stringdata0[198];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_F0ReadPage_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_F0ReadPage_t qt_meta_stringdata_F0ReadPage = {
    {
QT_MOC_LITERAL(0, 0, 10), // "F0ReadPage"
QT_MOC_LITERAL(1, 11, 8), // "onSendF0"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 17), // "onAutoPollToggled"
QT_MOC_LITERAL(4, 39, 7), // "checked"
QT_MOC_LITERAL(5, 47, 7), // "onClear"
QT_MOC_LITERAL(6, 55, 14), // "onDataReceived"
QT_MOC_LITERAL(7, 70, 4), // "data"
QT_MOC_LITERAL(8, 75, 11), // "onPollTimer"
QT_MOC_LITERAL(9, 87, 12), // "onBrowseFile"
QT_MOC_LITERAL(10, 100, 10), // "onLoadFile"
QT_MOC_LITERAL(11, 111, 13), // "onPlayToggled"
QT_MOC_LITERAL(12, 125, 11), // "onPrevFrame"
QT_MOC_LITERAL(13, 137, 11), // "onNextFrame"
QT_MOC_LITERAL(14, 149, 11), // "onPlayTimer"
QT_MOC_LITERAL(15, 161, 13), // "onFontChanged"
QT_MOC_LITERAL(16, 175, 5), // "index"
QT_MOC_LITERAL(17, 181, 16) // "onSaveRawToggled"

    },
    "F0ReadPage\0onSendF0\0\0onAutoPollToggled\0"
    "checked\0onClear\0onDataReceived\0data\0"
    "onPollTimer\0onBrowseFile\0onLoadFile\0"
    "onPlayToggled\0onPrevFrame\0onNextFrame\0"
    "onPlayTimer\0onFontChanged\0index\0"
    "onSaveRawToggled"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_F0ReadPage[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   79,    2, 0x08 /* Private */,
       3,    1,   80,    2, 0x08 /* Private */,
       5,    0,   83,    2, 0x08 /* Private */,
       6,    1,   84,    2, 0x08 /* Private */,
       8,    0,   87,    2, 0x08 /* Private */,
       9,    0,   88,    2, 0x08 /* Private */,
      10,    0,   89,    2, 0x08 /* Private */,
      11,    1,   90,    2, 0x08 /* Private */,
      12,    0,   93,    2, 0x08 /* Private */,
      13,    0,   94,    2, 0x08 /* Private */,
      14,    0,   95,    2, 0x08 /* Private */,
      15,    1,   96,    2, 0x08 /* Private */,
      17,    1,   99,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QByteArray,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   16,
    QMetaType::Void, QMetaType::Bool,    4,

       0        // eod
};

void F0ReadPage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<F0ReadPage *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onSendF0(); break;
        case 1: _t->onAutoPollToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->onClear(); break;
        case 3: _t->onDataReceived((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 4: _t->onPollTimer(); break;
        case 5: _t->onBrowseFile(); break;
        case 6: _t->onLoadFile(); break;
        case 7: _t->onPlayToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->onPrevFrame(); break;
        case 9: _t->onNextFrame(); break;
        case 10: _t->onPlayTimer(); break;
        case 11: _t->onFontChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->onSaveRawToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject F0ReadPage::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_F0ReadPage.data,
    qt_meta_data_F0ReadPage,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *F0ReadPage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *F0ReadPage::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_F0ReadPage.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int F0ReadPage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 13;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE

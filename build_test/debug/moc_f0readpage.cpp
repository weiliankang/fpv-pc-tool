/****************************************************************************
** Meta object code from reading C++ file 'f0readpage.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../f0readpage.h"
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
    QByteArrayData data[26];
    char stringdata0[268];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_F0ReadPage_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_F0ReadPage_t qt_meta_stringdata_F0ReadPage = {
    {
QT_MOC_LITERAL(0, 0, 10), // "F0ReadPage"
QT_MOC_LITERAL(1, 11, 13), // "f0CommandSent"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 11), // "freqUpdated"
QT_MOC_LITERAL(4, 38, 4), // "band"
QT_MOC_LITERAL(5, 43, 7), // "channel"
QT_MOC_LITERAL(6, 51, 3), // "hop"
QT_MOC_LITERAL(7, 55, 12), // "powerUpdated"
QT_MOC_LITERAL(8, 68, 6), // "pwrIdx"
QT_MOC_LITERAL(9, 75, 6), // "bitmap"
QT_MOC_LITERAL(10, 82, 8), // "onSendF0"
QT_MOC_LITERAL(11, 91, 17), // "onAutoPollToggled"
QT_MOC_LITERAL(12, 109, 7), // "checked"
QT_MOC_LITERAL(13, 117, 7), // "onClear"
QT_MOC_LITERAL(14, 125, 14), // "onDataReceived"
QT_MOC_LITERAL(15, 140, 4), // "data"
QT_MOC_LITERAL(16, 145, 11), // "onPollTimer"
QT_MOC_LITERAL(17, 157, 12), // "onBrowseFile"
QT_MOC_LITERAL(18, 170, 10), // "onLoadFile"
QT_MOC_LITERAL(19, 181, 13), // "onPlayToggled"
QT_MOC_LITERAL(20, 195, 11), // "onPrevFrame"
QT_MOC_LITERAL(21, 207, 11), // "onNextFrame"
QT_MOC_LITERAL(22, 219, 11), // "onPlayTimer"
QT_MOC_LITERAL(23, 231, 13), // "onFontChanged"
QT_MOC_LITERAL(24, 245, 5), // "index"
QT_MOC_LITERAL(25, 251, 16) // "onSaveRawToggled"

    },
    "F0ReadPage\0f0CommandSent\0\0freqUpdated\0"
    "band\0channel\0hop\0powerUpdated\0pwrIdx\0"
    "bitmap\0onSendF0\0onAutoPollToggled\0"
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
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   94,    2, 0x06 /* Public */,
       3,    3,   95,    2, 0x06 /* Public */,
       7,    2,  102,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    0,  107,    2, 0x08 /* Private */,
      11,    1,  108,    2, 0x08 /* Private */,
      13,    0,  111,    2, 0x08 /* Private */,
      14,    1,  112,    2, 0x08 /* Private */,
      16,    0,  115,    2, 0x08 /* Private */,
      17,    0,  116,    2, 0x08 /* Private */,
      18,    0,  117,    2, 0x08 /* Private */,
      19,    1,  118,    2, 0x08 /* Private */,
      20,    0,  121,    2, 0x08 /* Private */,
      21,    0,  122,    2, 0x08 /* Private */,
      22,    0,  123,    2, 0x08 /* Private */,
      23,    1,  124,    2, 0x08 /* Private */,
      25,    1,  127,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int,    4,    5,    6,
    QMetaType::Void, QMetaType::Int, QMetaType::UInt,    8,    9,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   12,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QByteArray,   15,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   12,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   24,
    QMetaType::Void, QMetaType::Bool,   12,

       0        // eod
};

void F0ReadPage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<F0ReadPage *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->f0CommandSent(); break;
        case 1: _t->freqUpdated((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: _t->powerUpdated((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 3: _t->onSendF0(); break;
        case 4: _t->onAutoPollToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->onClear(); break;
        case 6: _t->onDataReceived((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 7: _t->onPollTimer(); break;
        case 8: _t->onBrowseFile(); break;
        case 9: _t->onLoadFile(); break;
        case 10: _t->onPlayToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 11: _t->onPrevFrame(); break;
        case 12: _t->onNextFrame(); break;
        case 13: _t->onPlayTimer(); break;
        case 14: _t->onFontChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: _t->onSaveRawToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (F0ReadPage::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&F0ReadPage::f0CommandSent)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (F0ReadPage::*)(int , int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&F0ReadPage::freqUpdated)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (F0ReadPage::*)(int , quint32 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&F0ReadPage::powerUpdated)) {
                *result = 2;
                return;
            }
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
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 16;
    }
    return _id;
}

// SIGNAL 0
void F0ReadPage::f0CommandSent()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void F0ReadPage::freqUpdated(int _t1, int _t2, int _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void F0ReadPage::powerUpdated(int _t1, quint32 _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE

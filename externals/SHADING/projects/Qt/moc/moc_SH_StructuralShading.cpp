/****************************************************************************
** Meta object code from reading C++ file 'SH_StructuralShading.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.11.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/SH_StructuralShading.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SH_StructuralShading.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.11.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SH__StructuralShading_t {
    QByteArrayData data[5];
    char stringdata0[65];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SH__StructuralShading_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SH__StructuralShading_t qt_meta_stringdata_SH__StructuralShading = {
    {
QT_MOC_LITERAL(0, 0, 21), // "SH::StructuralShading"
QT_MOC_LITERAL(1, 22, 8), // "progress"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 8), // "finished"
QT_MOC_LITERAL(4, 41, 23) // "calculateShadingFactors"

    },
    "SH::StructuralShading\0progress\0\0"
    "finished\0calculateShadingFactors"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SH__StructuralShading[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,
       3,    0,   32,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    0,   33,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Double,    1,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void SH::StructuralShading::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        StructuralShading *_t = static_cast<StructuralShading *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->progress((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 1: _t->finished(); break;
        case 2: _t->calculateShadingFactors(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (StructuralShading::*)(double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StructuralShading::progress)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (StructuralShading::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StructuralShading::finished)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SH::StructuralShading::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_SH__StructuralShading.data,
      qt_meta_data_SH__StructuralShading,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *SH::StructuralShading::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SH::StructuralShading::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SH__StructuralShading.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SH::StructuralShading::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void SH::StructuralShading::progress(double _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SH::StructuralShading::finished()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE

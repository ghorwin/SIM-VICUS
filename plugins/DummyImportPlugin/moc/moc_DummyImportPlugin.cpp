/****************************************************************************
** Meta object code from reading C++ file 'DummyImportPlugin.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../src/DummyImportPlugin.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qplugin.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DummyImportPlugin.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DummyImportPlugin_t {
    QByteArrayData data[1];
    char stringdata0[18];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DummyImportPlugin_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DummyImportPlugin_t qt_meta_stringdata_DummyImportPlugin = {
    {
QT_MOC_LITERAL(0, 0, 17) // "DummyImportPlugin"

    },
    "DummyImportPlugin"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DummyImportPlugin[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

void DummyImportPlugin::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject DummyImportPlugin::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_DummyImportPlugin.data,
    qt_meta_data_DummyImportPlugin,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DummyImportPlugin::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DummyImportPlugin::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DummyImportPlugin.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "SVImportPluginInterface"))
        return static_cast< SVImportPluginInterface*>(this);
    if (!strcmp(_clname, "ibk.sim-vicus.Plugin.ImportInterface/1.0"))
        return static_cast< SVImportPluginInterface*>(this);
    return QObject::qt_metacast(_clname);
}

int DummyImportPlugin::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    return _id;
}

QT_PLUGIN_METADATA_SECTION
static constexpr unsigned char qt_pluginMetaData[] = {
    'Q', 'T', 'M', 'E', 'T', 'A', 'D', 'A', 'T', 'A', ' ', '!',
    // metadata version, Qt version, architectural requirements
    0, QT_VERSION_MAJOR, QT_VERSION_MINOR, qPluginArchRequirements(),
    0xbf, 
    // "IID"
    0x02,  0x78,  0x24,  'i',  'b',  'k',  '.',  's', 
    'i',  'm',  '-',  'v',  'i',  'c',  'u',  's', 
    '.',  'P',  'l',  'u',  'g',  'i',  'n',  '.', 
    'I',  'm',  'p',  'o',  'r',  't',  'I',  'n', 
    't',  'e',  'r',  'f',  'a',  'c',  'e', 
    // "className"
    0x03,  0x71,  'D',  'u',  'm',  'm',  'y',  'I', 
    'm',  'p',  'o',  'r',  't',  'P',  'l',  'u', 
    'g',  'i',  'n', 
    // "MetaData"
    0x04,  0xa7,  0x6b,  'a',  'b',  'i',  '-',  'v', 
    'e',  'r',  's',  'i',  'o',  'n',  0x63,  '0', 
    '.',  '9',  0x66,  'a',  'u',  't',  'h',  'o', 
    'r',  0x6f,  'A',  'n',  'd',  'r',  'e',  'a', 
    's',  ' ',  'N',  'i',  'c',  'o',  'l',  'a', 
    'i',  0x67,  'l',  'i',  'c',  'e',  'n',  's', 
    'e',  0x6f,  'L',  'G',  'P',  'L',  ' ',  '2', 
    ' ',  'o',  'r',  ' ',  'n',  'e',  'w',  'e', 
    'r',  0x70,  'l',  'o',  'n',  'g',  '-',  'd', 
    'e',  's',  'c',  'r',  'i',  'p',  't',  'i', 
    'o',  'n',  0x79,  0x01,  0x05,  'e',  'n',  ':', 
    'T',  'h',  'i',  's',  ' ',  's',  'm',  'a', 
    'l',  'l',  ' ',  'd',  'u',  'm',  'm',  'y', 
    ' ',  'i',  'm',  'p',  'o',  'r',  't',  ' ', 
    'p',  'l',  'u',  'g',  'i',  'n',  ' ',  'i', 
    'l',  'l',  'u',  's',  't',  'r',  'a',  't', 
    'e',  's',  ' ',  't',  'h',  'e',  ' ',  'u', 
    's',  'e',  ' ',  'o',  'f',  ' ',  't',  'h', 
    'e',  ' ',  'i',  'm',  'p',  'o',  'r',  't', 
    ' ',  'i',  'n',  't',  'e',  'r',  'f',  'a', 
    'c',  'e',  ' ',  'b',  'y',  ' ',  'g',  'e', 
    'n',  'e',  'r',  'a',  't',  'i',  'n',  'g', 
    ' ',  'n',  'o',  ' ',  'd',  'a',  't',  'a', 
    ',',  ' ',  'b',  'u',  't',  ' ',  's',  'i', 
    'm',  'p',  'l',  'y',  ' ',  's',  'h',  'o', 
    'w',  'i',  'n',  'g',  ' ',  't',  'h',  'e', 
    ' ',  'i',  'm',  'p',  'o',  'r',  't',  ' ', 
    'd',  'i',  'a',  'l',  'o',  'g',  '.',  '|', 
    'd',  'e',  ':',  'D',  'i',  'e',  's',  'e', 
    's',  ' ',  'k',  'l',  'e',  'i',  'n',  'e', 
    ' ',  'D',  'u',  'm',  'm',  'y',  '-',  'I', 
    'm',  'p',  'o',  'r',  't',  '-',  'P',  'l', 
    'u',  'g',  'i',  'n',  ' ',  'm',  'a',  'c', 
    'h',  't',  ' ',  'n',  'i',  'c',  'h',  't', 
    ' ',  'w',  'i',  'r',  'k',  'l',  'i',  'c', 
    'h',  ' ',  'w',  'a',  's',  ',',  ' ',  'z', 
    'e',  'i',  'g',  't',  ' ',  'a',  'b',  'e', 
    'r',  ',',  ' ',  'w',  'i',  'e',  ' ',  'm', 
    'a',  'n',  ' ',  'd',  'i',  'e',  ' ',  'I', 
    'm',  'p',  'o',  'r',  't',  '-',  'S',  'c', 
    'h',  'n',  'i',  't',  't',  's',  't',  'e', 
    'l',  'l',  'e',  ' ',  'i',  'm',  'p',  'l', 
    'e',  'm',  'e',  'n',  't',  'i',  'e',  'r', 
    't',  '.',  0x71,  's',  'h',  'o',  'r',  't', 
    '-',  'd',  'e',  's',  'c',  'r',  'i',  'p', 
    't',  'i',  'o',  'n',  0x78,  0x52,  'e',  'n', 
    ':',  'T',  'h',  'i',  's',  ' ',  'i',  's', 
    ' ',  'a',  ' ',  'd',  'u',  'm',  'm',  'y', 
    ' ',  'i',  'm',  'p',  'o',  'r',  't',  ' ', 
    'p',  'l',  'u',  'g',  'i',  'n',  '.',  '|', 
    'd',  'e',  ':',  'D',  'i',  'e',  's',  ' ', 
    'i',  's',  't',  ' ',  'e',  'i',  'n',  ' ', 
    'B',  'e',  'i',  's',  'p',  'i',  'e',  'l', 
    ' ',  'f',  uchar('\xc3'), uchar('\xbc'), 'r',  ' ',  'e',  'i', 
    'n',  ' ',  'I',  'm',  'p',  'o',  'r',  't', 
    '-',  'P',  'l',  'u',  'g',  'i',  'n',  '.', 
    0x67,  'v',  'e',  'r',  's',  'i',  'o',  'n', 
    0x65,  '1',  '.',  '0',  '.',  '0',  0x67,  'w', 
    'e',  'b',  'p',  'a',  'g',  'e',  0x74,  'h', 
    't',  't',  'p',  's',  ':',  '/',  '/',  's', 
    'i',  'm',  '-',  'v',  'i',  'c',  'u',  's', 
    '.',  'd',  'e', 
    0xff, 
};
QT_MOC_EXPORT_PLUGIN(DummyImportPlugin, DummyImportPlugin)

QT_WARNING_POP
QT_END_MOC_NAMESPACE

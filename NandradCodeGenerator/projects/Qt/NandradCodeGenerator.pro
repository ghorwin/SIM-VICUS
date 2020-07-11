# Project file for NandradCodeGenerator

TARGET = NandradCodeGenerator
TEMPLATE = app
QT -= core gui
CONFIG += console c++11
CONFIG -= app_bundle

INCLUDEPATH = \
	../../../externals/IBK/src

LIBS += \
	-L../../../externals/lib_x64 \
	-lIBK

unix|mac {
	QMAKE_CXXFLAGS +=  -std=c++11
}

DEPENDPATH = $${INCLUDEPATH}

SOURCES += \
	../../src/ClassInfo.cpp \
	../../src/main.cpp
HEADERS += \
	../../src/ClassInfo.h

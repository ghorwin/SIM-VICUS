# Project file for NandradFMUGenerator
#
# remember to set DYLD_FALLBACK_LIBRARY_PATH on MacOSX
# set LD_LIBRARY_PATH on Linux

TARGET = NandradFMUGenerator
TEMPLATE = app

# this pri must be sourced from all our applications
include( ../../../externals/IBK/projects/Qt/IBK.pri )

QT += widgets

CONFIG += c++11

LIBS += -L../../../lib$${DIR_PREFIX} \
	-lNandrad \
	-lQtExt \
	-lQuaZIP \
	-lTiCPP \
	-lIBK

INCLUDEPATH = \
	../../src \
	../../../externals/IBK/src \
	../../../externals/IBKMK/src \
	../../../externals/Nandrad/src \
	../../../externals/QuaZIP/src \
	../../../externals/QuaZIP/src/zlib \
	../../../externals/QtExt/src

DEPENDPATH = $${INCLUDEPATH}

SOURCES +=  \
	../../src/FMUVariableTableModel.cpp \
	../../src/NandradFMUGeneratorWidget.cpp \
	../../src/main.cpp

FORMS += \
	../../src/NandradFMUGeneratorWidget.ui

HEADERS += \
	../../src/FMUVariableTableModel.h \
	../../src/NandradFMUGeneratorWidget.h

RESOURCES += \
	../../resources/NandradFMUGenerator.qrc


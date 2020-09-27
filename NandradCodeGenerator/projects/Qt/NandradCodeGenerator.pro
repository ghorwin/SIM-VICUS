# Project file for NandradCodeGenerator

TARGET = NandradCodeGenerator
TEMPLATE = app

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../externals/IBK/projects/Qt/IBK.pri )


INCLUDEPATH = \
	../../../externals/IBK/src

LIBS += \
	-lIBK

DEPENDPATH = $${INCLUDEPATH}

win32 {
	PRE_TARGETDEPS +=   $$PWD/../../../externals/lib$${DIR_PREFIX}/IBK.lib
}

SOURCES += \
	../../src/ClassInfo.cpp \
	../../src/CodeGenerator.cpp \
	../../src/constants.cpp \
	../../src/main.cpp
HEADERS += \
	../../src/ClassInfo.h \
	../../src/CodeGenerator.h \
	../../src/constants.h

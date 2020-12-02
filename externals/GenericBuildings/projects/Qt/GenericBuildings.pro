TARGET = GenericBuildings
TEMPLATE = app

include( ../../../externals/IBK/projects/Qt/IBK.pri )

CONFIG += console c++11
CONFIG -= app_bundle
CONFIG += qt


LIBS += \
		-lQtExt \
		-lTiCPP \
		-lIBKMK \
		-lIBK


INCLUDEPATH = \
				../../../externals/IBK/src \
		../../../externals/IBKMK/src \
		../../../externals/QtExt/src

SOURCES += 

# remember to set DYLD_FALLBACK_LIBRARY_PATH on MacOSX
# set LD_LIBRARY_PATH on Linux

TARGET = SIM-VICUS
TEMPLATE = app

# this pri must be sourced from all our applications
include( ../../../externals/IBK/projects/Qt/IBK.pri )

QT += xml core gui network printsupport widgets

LIBS += -L../../../lib$${DIR_PREFIX} \
	-lNandrad \
	-lminizip \
	-lIBK \
	-lTiCPP

win32 {
	LIBS += -lzlib -luser32
}

INCLUDEPATH = \
	../../src \
	../../../externals/zlib/src \
	../../../externals/minizip/src \
	../../../externals/IBK/src \
	../../../externals/Nandrad/src \
	../../../externals/TiCPP/src 


SOURCES += \
	../../src/main.cpp 

HEADERS  += 

FORMS    += 

TRANSLATIONS += ../../resources/translations/SIM-VICUS_de.ts
CODECFORSRC = UTF-8

RESOURCES += \
	../../resources/SIM-VICUS.qrc


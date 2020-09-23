# Project file for SIM-VICUS
#
# remember to set DYLD_FALLBACK_LIBRARY_PATH on MacOSX
# set LD_LIBRARY_PATH on Linux

TARGET = SIM-VICUS
TEMPLATE = app

# this pri must be sourced from all our applications
include( ../../../externals/IBK/projects/Qt/IBK.pri )

QT += xml opengl network printsupport widgets

unix {
	QMAKE_CXXFLAGS += -std=c++11
}

LIBS += -L../../../lib$${DIR_PREFIX} \
	-lNandrad \
	-lIBK \
	-lTiCPP \
	-lQuaZIP

win32 {
	LIBS += -luser32
	LIBS += -lopengl32
}
linux {
	LIBS += -lGLU -lGL
}
mac {
	LIBS += -framework OpenGL
}

INCLUDEPATH = \
	../../src \
	../../../externals/IBK/src \
	../../../externals/Nandrad/src \
	../../../externals/TiCPP/src

win32 {
	PRE_TARGETDEPS +=   $$PWD/../../../externals/lib$${DIR_PREFIX}/IBK.lib \
						$$PWD/../../../externals/lib$${DIR_PREFIX}/CCM.lib \
						$$PWD/../../../externals/lib$${DIR_PREFIX}/qwt6.lib \
						$$PWD/../../../externals/lib$${DIR_PREFIX}/QuaZIP.lib \
						$$PWD/../../../externals/lib$${DIR_PREFIX}/Nandrad.lib \
						$$PWD/../../../externals/lib$${DIR_PREFIX}/TiCPP.lib
}


SOURCES += \
	../../src/main.cpp

HEADERS  +=

FORMS    +=

TRANSLATIONS += ../../resources/translations/SIM-VICUS_de.ts
CODECFORSRC = UTF-8

RESOURCES += ../../resources/SIM-VICUS.qrc



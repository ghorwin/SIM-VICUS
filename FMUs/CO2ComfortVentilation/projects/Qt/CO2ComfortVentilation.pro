# ----------------------------------
# Qt Project for building FMU 
# ----------------------------------
#
# This file is part of FMICodeGenerator (https://github.com/ghorwin/FMICodeGenerator)
# 
# BSD 3-Clause License
#
# Copyright (c) 2018, Andreas Nicolai
# All rights reserved.
#
# see https://github.com/ghorwin/FMICodeGenerator/blob/master/LICENSE for details.


TARGET = CO2ComfortVentilation
TEMPLATE = lib

# no GUI
QT -= core gui

CONFIG(debug, debug|release) {
	windows {
		DLLDESTDIR = ../../bin/debug$${DIR_PREFIX}
	}
	else {
		DESTDIR = ../../bin/debug$${DIR_PREFIX}
	}
}
else {
	windows {
		DLLDESTDIR = ../../bin/release$${DIR_PREFIX}
	}
	else {
		DESTDIR = ../../bin/release$${DIR_PREFIX}
	}
}

#DEFINES += FMI2_FUNCTION_PREFIX=CO2ComfortVentilation_

unix|mac {
	VER_MAJ = 1
	VER_MIN = 0
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

unix {
        QMAKE_POST_LINK += $$quote(mv $${DESTDIR}/lib$${TARGET}.so $${DESTDIR}/$${TARGET}.so)
}

INCLUDEPATH = ../../src

SOURCES += \
	../../src/Helpers.cpp \
	../../src/LinearSpline.cpp \
	../../src/Path.cpp \
	../../src/fmi2common/fmi2Functions.cpp \
	../../src/fmi2common/InstanceData.cpp \
	../../src/CO2ComfortVentilation.cpp

HEADERS += \
	../../src/Helpers.h \
	../../src/LinearSpline.h \
	../../src/Path.h \
	../../src/fmi2common/fmi2Functions.h \
	../../src/fmi2common/fmi2Functions_complete.h \
	../../src/fmi2common/fmi2FunctionTypes.h \
	../../src/fmi2common/fmi2TypesPlatform.h \
	../../src/fmi2common/InstanceData.h \
	../../src/CO2ComfortVentilation.h



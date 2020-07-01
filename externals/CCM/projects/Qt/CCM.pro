# -------------------------------------------------
# Project for CCM library
# -------------------------------------------------

# first we define what we are
TARGET = CCM
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )


# finally we setup our custom library specfic things
# like version number etc., we also may reset all
unix|mac {
	VER_MAJ = 1
	VER_MIN = 0
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

LIBS += -lIBK -lTiCPP

INCLUDEPATH +=	\
	../../../IBK/src \
	../../../TiCPP/src

SOURCES += \
	../../src/CCM_ClimateDataLoader.cpp \
	../../src/CCM_SolarRadiationModel.cpp \
	../../src/CCM_Constants.cpp \
	../../src/CCM_SunPositionModel.cpp
HEADERS += \
	../../src/CCM_ClimateDataLoader.h \
	../../src/CCM_SolarRadiationModel.h \
	../../src/CCM_Constants.h \
	../../src/CCM_SunPositionModel.h \
	../../src/CCM_Defines.h

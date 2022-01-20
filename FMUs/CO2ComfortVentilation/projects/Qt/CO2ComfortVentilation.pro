# ------------------------------------------
# Qt Project for CO2 confort ventilation FMU
# ------------------------------------------

TARGET        = CO2ComfortVentilation
TEMPLATE      = lib

# this pri must be sourced from all our applications
include( ../../../Qt/fmus.pri )

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
	../../src/fast_double_parser/fast_double_parser.h \
	../../src/fmi2common/fmi2Functions.h \
	../../src/fmi2common/fmi2Functions_complete.h \
	../../src/fmi2common/fmi2FunctionTypes.h \
	../../src/fmi2common/fmi2TypesPlatform.h \
	../../src/fmi2common/InstanceData.h \
	../../src/CO2ComfortVentilation.h



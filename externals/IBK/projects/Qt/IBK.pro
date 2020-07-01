# -----------------------
# Project for IBK library
# -----------------------

# first we define what we are
TARGET = IBK
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )

# finally we setup our custom library specfic things
# like version number etc., we also may reset all
#
unix|mac {
	VER_MAJ = 5
	VER_MIN = 0
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

SOURCES += \
	../../src/IBK_ArgParser.cpp \
	../../src/IBK_Color.cpp \
	../../src/IBK_Constants.cpp \
	../../src/IBK_crypt.cpp \
	../../src/IBK_EOLStreamBuffer.cpp \
	../../src/IBK_Exception.cpp \
	../../src/IBK_FileReader.cpp \
	../../src/IBK_FileUtils.cpp \
	../../src/IBK_Flag.cpp \
	../../src/IBK_FormatString.cpp \
	../../src/IBK_InputOutput.cpp \
	../../src/IBK_Line.cpp \
	../../src/IBK_LinearSpline.cpp \
	../../src/IBK_Logfile.cpp \
	../../src/IBK_geographic.cpp \
	../../src/IBK_math.cpp \
	../../src/IBK_messages.cpp \
	../../src/IBK_MultiLanguageString.cpp \
	../../src/IBK_Parameter.cpp \
	../../src/IBK_Quantity.cpp \
	../../src/IBK_QuantityManager.cpp \
	../../src/IBK_SolverArgsParser.cpp \
	../../src/IBK_StopWatch.cpp \
	../../src/IBK_StringUtils.cpp \
	../../src/IBK_Time.cpp \
	../../src/IBK_Unit.cpp \
	../../src/IBK_UnitList.cpp \
	../../src/IBK_UnitVector.cpp \
	../../src/IBK_MessageHandlerRegistry.cpp \
	../../src/IBK_MessageHandler.cpp \
	../../src/IBK_Path.cpp \
	../../src/IBK_CSVReader.cpp \
	../../src/IBK_physics.cpp \
	../../src/IBK_Version.cpp \
	../../src/IBK_system.cpp \
	../../src/IBK_IntPara.cpp

OTHER_FILES +=

HEADERS += \
	../../src/IBK_algorithm.h \
	../../src/IBK_ArgParser.h \
	../../src/IBK_array.h \
	../../src/IBK_assert.h \
	../../src/IBK_bitfield.h \
	../../src/IBK_BuildFlags.h \
	../../src/IBK_Color.h \
	../../src/IBK_configuration.h \
	../../src/IBK_configuration.h.in \
	../../src/IBK_Constants.h \
	../../src/IBK_crypt.h \
	../../src/IBK_cuboid.h \
	../../src/IBK_EOLStreamBuffer.h \
	../../src/IBK_Exception.h \
	../../src/IBK_FileReader.h \
	../../src/IBK_FileUtils.h \
	../../src/IBK_Flag.h \
	../../src/IBK_FormatString.h \
	../../src/IBK_InputOutput.h \
	../../src/IBK_Line.h \
	../../src/IBK_LinearSpline.h \
	../../src/IBK_Logfile.h \
	../../src/IBK_geographic.h \
	../../src/IBK_math.h \
	../../src/IBK_matrix_view.h \
	../../src/IBK_memory_usage.h \
	../../src/IBK_messages.h \
	../../src/IBK_MultiLanguageString.h \
	../../src/IBK_NotificationHandler.h \
	../../src/IBK_Parameter.h \
	../../src/IBK_Path.h \
	../../src/IBK_physics.h \
	../../src/IBK_point.h \
	../../src/IBK_ptr_list.h \
	../../src/IBK_Quantity.h \
	../../src/IBK_QuantityManager.h \
	../../src/IBK_rectangle.h \
	../../src/IBK_SimpleString.h \
	../../src/IBK_SolverArgsParser.h \
	../../src/IBK_StopWatch.h \
	../../src/IBK_StringUtils.h \
	../../src/IBK_Time.h \
	../../src/IBK_Unit.h \
	../../src/IBK_UnitData.h \
	../../src/IBK_UnitList.h \
	../../src/IBK_UnitVector.h \
	../../src/IBK_WaitOnExit.h \
	../../src/IBK_matrix.h \
	../../src/IBK_matrix_3d.h \
	../../src/IBK_MessageHandlerRegistry.h \
	../../src/IBK_MessageHandler.h \
	../../src/IBK_ScalarFunction.h \
	../../src/IBK_CSVReader.h \
	../../src/IBK_openMP.h \
	../../src/IBK_Version.h \
	../../src/IBK_system.h \
	../../src/IBK_IntPara.h

DISTFILES += \
	../../doc/LICENSE


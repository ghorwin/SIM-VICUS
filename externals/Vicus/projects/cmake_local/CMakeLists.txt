# CMakeLists.txt file for Sim-Vicus data model library
#
# Note: This CMakeLists.txt file is not for stand-alone builds.
#       Requires toplevel CMakeLists.txt file that defines the variable
#       KEYWORDLISTCREATOR.

project( Vicus )

include_directories(
	${PROJECT_SOURCE_DIR}/../../src
	${PROJECT_SOURCE_DIR}/../../../CCM/src
	${PROJECT_SOURCE_DIR}/../../../IBK/src
	${PROJECT_SOURCE_DIR}/../../../IBKMK/src
	${PROJECT_SOURCE_DIR}/../../../TiCPP/src
	${PROJECT_SOURCE_DIR}/../../../Nandrad/src
	${PROJECT_SOURCE_DIR}/../../../CCM/src
	${Qt5Widgets_INCLUDE_DIRS}
)

# collect a list of all source files of the Vicus library
file( GLOB Vicus_LIB_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )
file( GLOB Vicus_NCG_LIB_SRCS ${PROJECT_SOURCE_DIR}/../../src/ncg/*.cpp )

# add VICUS_KeywordList.cpp
set( Vicus_LIB_SRCS
	${Vicus_LIB_SRCS}
	${Vicus_NCG_LIB_SRCS}
	${PROJECT_SOURCE_DIR}/../../src/VICUS_KeywordList.cpp
)

# collect a list of all header files of the Vicus library
file( GLOB Vicus_HDRS ${PROJECT_SOURCE_DIR}/../../src/*.h )

# run the KeywordListCreator tool whenever the header files have changed
# to update the VICUS_KeywordList.h and VICUS_KeywordList.cpp file
add_custom_command (
	OUTPUT   ${PROJECT_SOURCE_DIR}/../../src/VICUS_KeywordList.cpp
	DEPENDS  ${Vicus_HDRS} NandradCodeGenerator
	COMMAND  NandradCodeGenerator
	ARGS     VICUS ${PROJECT_SOURCE_DIR}/../../src 1 VICUS ncg
)

# build the library
add_library( Vicus STATIC
	${Vicus_LIB_SRCS}
)


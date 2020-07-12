# CMakeLists.txt file for Nandrad library
#
# Note: This CMakeLists.txt file is not for stand-alone builds.
#       Requires toplevel CMakeLists.txt file that defines the variable
#       KEYWORDLISTCREATOR.

project( Nandrad )

include_directories(
	${PROJECT_SOURCE_DIR}/../../../IBK/src
	${PROJECT_SOURCE_DIR}/../../../IBKMK/src
	${PROJECT_SOURCE_DIR}/../../../TiCPP/src
)

# collect a list of all source files of the Nandrad library
file( GLOB Nandrad_LIB_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )
file( GLOB Nandrad_NCG_LIB_SRCS ${PROJECT_SOURCE_DIR}/../../src/ncg/*.cpp )

# add NANDRAD_KeywordList.cpp
set( Nandrad_LIB_SRCS
	${Nandrad_LIB_SRCS}
	${Nandrad_NCG_LIB_SRCS}
	${PROJECT_SOURCE_DIR}/../../src/NANDRAD_KeywordList.cpp
)

# collect a list of all header files of the Nandrad library
file( GLOB Nandrad_HDRS ${PROJECT_SOURCE_DIR}/../../src/*.h )

# run the KeywordListCreator tool whenever the header files have changed
# to update the NANDRAD_KeywordList.h and NANDRAD_KeywordList.cpp file
add_custom_command (
	OUTPUT   ${PROJECT_SOURCE_DIR}/../../src/NANDRAD_KeywordList.cpp
	DEPENDS  ${Nandrad_HDRS} NandradCodeGenerator
	COMMAND  NandradCodeGenerator
	ARGS     NANDRAD ${PROJECT_SOURCE_DIR}/../../src 0 NANDRAD ncg
)


# build the library
add_library( Nandrad STATIC
	${Nandrad_LIB_SRCS}
)


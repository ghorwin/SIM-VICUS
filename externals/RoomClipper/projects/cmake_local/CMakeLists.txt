# CMakeLists.txt file for RoomClipper library

project(RoomClipper)

# collect a list of all source files of the RoomClipper library
file (GLOB LIB_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )
SET( LIB_SRCS ${LIB_SRCS} )

include_directories(
	${PROJECT_SOURCE_DIR}/../../../clipper/src
	${PROJECT_SOURCE_DIR}/../../../IBK/src
	${PROJECT_SOURCE_DIR}/../../../IBKMK/src
	${PROJECT_SOURCE_DIR}/../../../CCM/src
	${PROJECT_SOURCE_DIR}/../../../NANDRAD/src
	${PROJECT_SOURCE_DIR}/../../../DATAIO/src
	${PROJECT_SOURCE_DIR}/../../../VICUS/src
	${PROJECT_SOURCE_DIR}/../../../TiCPP/src
)

# build the library
add_library( ${PROJECT_NAME} STATIC
	${LIB_SRCS}
)


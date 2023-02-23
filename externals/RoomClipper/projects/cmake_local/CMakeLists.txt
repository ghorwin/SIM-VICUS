# CMakeLists.txt file for RoomClipper library

project(RoomClipper)

# collect a list of all source files of the RoomClipper library
file (GLOB LIB_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )

include_directories(
	${PROJECT_SOURCE_DIR}/../../../IBK/src
	${PROJECT_SOURCE_DIR}/../../../IBKMK/src
	${PROJECT_SOURCE_DIR}/../../../CCM/src
	${PROJECT_SOURCE_DIR}/../../../Nandrad/src
	${PROJECT_SOURCE_DIR}/../../../DATAIO/src
	${PROJECT_SOURCE_DIR}/../../../Vicus/src
	${PROJECT_SOURCE_DIR}/../../../TiCPP/src
        ${PROJECT_SOURCE_DIR}/../../../clipper/src
	${Qt5Widgets_INCLUDE_DIRS}
)

# build the library
add_library( ${PROJECT_NAME} STATIC
	${LIB_SRCS}
)

# CMakeLists.txt file for Shading library

project(Shading)

# collect a list of all source files of the library
file (GLOB LIB_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )

include_directories(
	${PROJECT_SOURCE_DIR}/../../../DataIO/src
	${PROJECT_SOURCE_DIR}/../../../IBK/src
	${PROJECT_SOURCE_DIR}/../../../IBKMK/src
	${PROJECT_SOURCE_DIR}/../../../clipper/src
	${PROJECT_SOURCE_DIR}/../../../CCM/src
	${PROJECT_SOURCE_DIR}/../../../TiCPP/srck
	${Qt5Gui_INCLUDE_DIRS}
)

# build the library
add_library( ${PROJECT_NAME} STATIC
	${LIB_SRCS}
)

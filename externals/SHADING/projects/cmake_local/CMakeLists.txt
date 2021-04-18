# CMakeLists.txt file for SHADING library

cmake_minimum_required( VERSION 2.6 )

project(SHADING)

# collect a list of all source files of the IBK library
file (GLOB LIB_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )

include_directories(
	${PROJECT_SOURCE_DIR}/../../../IBK/src
	${PROJECT_SOURCE_DIR}/../../../IBKMK/src
	${PROJECT_SOURCE_DIR}/../../../CCM/src
	${PROJECT_SOURCE_DIR}/../../../TiCPP/srck
	${Qt5Widgets_INCLUDE_DIRS}
)

# collect a list of all header files (to be used in MOC compiler)
file( GLOB LIB_HDRS ${PROJECT_SOURCE_DIR}/../../src/*.h )

# run uic and moc compilers
qt5_wrap_cpp( LIB_MOC_SRCS ${LIB_HDRS} )

# build the library
add_library( SHADING STATIC
	${LIB_SRCS}
	${LIB_MOC_SRCS}
)
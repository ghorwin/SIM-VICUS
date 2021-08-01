# CMakeLists.txt file for CCM library

cmake_minimum_required( VERSION 2.8...3.10 )

project(CCM)

# collect a list of all source files of the IBK library
file (GLOB LIB_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )

include_directories(
	${PROJECT_SOURCE_DIR}/../../../IBK/src
	${PROJECT_SOURCE_DIR}/../../../TiCPP/src
)

# build the library
add_library( CCM STATIC
	${LIB_SRCS}
)


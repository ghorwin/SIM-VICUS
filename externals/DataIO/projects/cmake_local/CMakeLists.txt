# CMakeLists.txt file for DataIO library

cmake_minimum_required( VERSION 2.6 )

project( DataIO )

# collect a list of all source files
file( GLOB LIB_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )

# add include directories
include_directories(
	${PROJECT_SOURCE_DIR}/../../../IBK/src 
)

# build the library
add_library( ${PROJECT_NAME} STATIC 
	${LIB_SRCS}
)


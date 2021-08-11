# CMakeLists.txt file for IDFReader library

project( IDFReader )

# collect a list of all source files
file( GLOB LIB_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )

# add include directories
include_directories(
	${PROJECT_SOURCE_DIR}/../../../IBK/src 
	${PROJECT_SOURCE_DIR}/../../../IBKMK/src 
)

# build the library
add_library( ${PROJECT_NAME} STATIC 
	${LIB_SRCS}
)


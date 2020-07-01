# CMakeLists.txt file for IBKMK library

project( IBKMK )

# add include directories
include_directories(
	${PROJECT_SOURCE_DIR}/../../../IBK/src
)

# collect a list of all source files of the IBK library
file( GLOB IBKMK_LIB_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )
file( GLOB IBKMK_LIB_C_SRCS  ${PROJECT_SOURCE_DIR}/../../src/*.c )
SET( IBKMK_LIB_SRCS ${IBKMK_LIB_SRCS} ${IBKMK_LIB_C_SRCS} )

# build the library
add_library( IBKMK STATIC
	${IBKMK_LIB_SRCS}
)

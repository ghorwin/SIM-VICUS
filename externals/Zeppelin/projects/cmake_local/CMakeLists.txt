# CMakeLists.txt file for Zeppelin library

project( Zeppelin )

# collect a list of all source files of the Zeppelin library
file( GLOB Zeppelin_LIB_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )
SET( Zeppelin_LIB_SRCS ${Zeppelin_LIB_SRCS} )

# build the library
add_library( Zeppelin STATIC
	${Zeppelin_LIB_SRCS}
)


# CMakeLists.txt file for IBK library

project( IBK )

# configuration file options
option( IBK_ENABLE_SAFE_MATH "Turns checking of function arguments to math routines on." ON )
option( IBK_ENABLE_UNIT_NAME "Adds a member variable with unit ID name to the IBK::Unit for better debugging." ON )
option( IBK_ENABLE_COLORED_CONSOLE "Enables colored outputs in the console window." ON )

# avoid warning about unknown pragmas
if( UNIX )
	add_definitions( -Wunknown-pragmas )
endif( UNIX )

configure_file(
	${PROJECT_SOURCE_DIR}/../../src/IBK_configuration.h.in
	${PROJECT_SOURCE_DIR}/../../src/IBK_configuration.h
)

# collect a list of all source files of the IBK library
file( GLOB IBK_LIB_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )
SET( IBK_LIB_SRCS ${IBK_LIB_SRCS} )

# build the library
add_library( IBK STATIC
	${IBK_LIB_SRCS}
)


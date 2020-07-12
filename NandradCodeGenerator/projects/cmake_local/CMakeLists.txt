project( NandradCodeGenerator )

if (CMAKE_COMPILER_IS_GNUCC)
	if (NOT APPLE)
		# link solver statically
		set( CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++" )
	endif (NOT APPLE)
endif (CMAKE_COMPILER_IS_GNUCC)

# add include directories
include_directories(
	${PROJECT_SOURCE_DIR}/../../src
	${PROJECT_SOURCE_DIR}/../../../externals/IBK/src
)


# set variable for dependent libraries
set( LINK_LIBS
	IBK
)

# collect a list of all source files of the library
file( GLOB APP_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )

add_executable( ${PROJECT_NAME}
	${APP_SRCS}
)

# link against the dependent libraries
target_link_libraries( ${PROJECT_NAME} 
	${LINK_LIBS}
)





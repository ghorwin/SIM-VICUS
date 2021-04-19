project( View3D C)

if (CMAKE_COMPILER_IS_GNUCC)
	if (NOT APPLE)
		# link solver statically
		set( CMAKE_EXE_LINKER_FLAGS "-static-libgcc" )
	endif (NOT APPLE)
endif (CMAKE_COMPILER_IS_GNUCC)

# add include directories
include_directories(
	${PROJECT_SOURCE_DIR}/../../src
)

if(MSVC)
  add_definitions( -D_CRT_SECURE_NO_DEPRECATE )
endif()

# collect a list of all header files of the library
#file( GLOB APP_HDRS ${PROJECT_SOURCE_DIR}/../../src/*.h )

# collect a list of all source files of the library
file( GLOB APP_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.c )

add_executable( ${PROJECT_NAME}
	${APP_SRCS}
)

# link against the dependent libraries
#target_link_libraries( ${PROJECT_NAME} 
#)


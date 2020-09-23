# CMakeLists.txt file for QuaZIP library

project( QuaZIP )

include_directories(
	${PROJECT_SOURCE_DIR}/../../src
	${PROJECT_SOURCE_DIR}/../../src/zlib
	${Qt5Widgets_INCLUDE_DIRS}
)

# collect a list of all source files of the QuaZIP library and zlib library
file( GLOB QuaZIP_LIB_CPP_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )
file( GLOB QuaZIP_LIB_C_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.c )
file( GLOB QuaZIP_LIB_ZLIB_SRCS ${PROJECT_SOURCE_DIR}/../../src/zlib/*.c )

# collect a list of all header files (to be used in MOC compiler)
file( GLOB LIB_HDRS ${PROJECT_SOURCE_DIR}/../../src/*.h )

# run headers through moc compiler
if (Qt5Widgets_INCLUDE_DIRS)

	qt5_wrap_cpp( LIB_MOC_SRCS ${LIB_HDRS} )

else (Qt5Widgets_INCLUDE_DIRS)

	qt4_wrap_cpp( LIB_MOC_SRCS ${LIB_HDRS} )

endif (Qt5Widgets_INCLUDE_DIRS)

# join sources into one variable
set( LIB_SRCS
	${QuaZIP_LIB_CPP_SRCS}
	${QuaZIP_LIB_C_SRCS}
	${QuaZIP_LIB_ZLIB_SRCS}
)

# add build target for library
add_library( ${PROJECT_NAME} STATIC
	${LIB_SRCS}
	${LIB_MOC_SRCS}
)


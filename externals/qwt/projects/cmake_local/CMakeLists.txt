# CMakeLists.txt file for qwt library
# Does not work as stand-alone CMakeLists.txt file, must be included in CMakeLists.txt master file.

project( qwt6 )

include_directories(
	${PROJECT_SOURCE_DIR}/../../src
	${Qt5Core_INCLUDE_DIRS}
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt5PrintSupport_INCLUDE_DIRS}
	${Qt5Svg_INCLUDE_DIRS}
	${Qt5Concurrent_INCLUDE_DIRS}
)

# collect a list of all source files of the solver
file( GLOB LIB_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )
# but don't build openGl plot canvas for now
list( REMOVE_ITEM LIB_SRCS ${PROJECT_SOURCE_DIR}/../../src/qwt_plot_glcanvas.cpp)

# collect a list of moc header-files in 
file( GLOB MOC_HDRS ${PROJECT_SOURCE_DIR}/../../src/*.h )
list( REMOVE_ITEM MOC_HDRS ${PROJECT_SOURCE_DIR}/../../src/qwt_plot_glcanvas.h)
list( REMOVE_ITEM MOC_HDRS ${PROJECT_SOURCE_DIR}/../../src/qwt_widget_overlay.h)

# run headers through moc compiler
if (Qt5Widgets_INCLUDE_DIRS)

	qt5_wrap_cpp( LIB_MOC_SRCS ${MOC_HDRS} )

else (Qt5Widgets_INCLUDE_DIRS)

	qt4_wrap_cpp( LIB_MOC_SRCS ${MOC_HDRS} )

endif (Qt5Widgets_INCLUDE_DIRS)

# add build target for library
add_library( ${PROJECT_NAME} STATIC
	${LIB_SRCS}
	${LIB_MOC_SRCS} 
)


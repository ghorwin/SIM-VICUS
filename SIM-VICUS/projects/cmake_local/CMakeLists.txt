# CMakeLists.txt file for SIM-VICUS

project( SIM-VICUS )

# add include directories
include_directories(
	${PROJECT_SOURCE_DIR}/../../src			# needed so that ui-generated header files find our own headers
	${PROJECT_SOURCE_DIR}/../../src/actions
	${PROJECT_SOURCE_DIR}/../../src/core3D
	${PROJECT_SOURCE_DIR}/../../../externals/IBK/src
	${PROJECT_SOURCE_DIR}/../../../externals/IBKMK/src
	${PROJECT_SOURCE_DIR}/../../../externals/Nandrad/src
	${PROJECT_SOURCE_DIR}/../../../externals/TiCPP/src
	${PROJECT_SOURCE_DIR}/../../../externals/Vicus/src
	${PROJECT_SOURCE_DIR}/../../../externals/QtExt/src
	${PROJECT_SOURCE_DIR}/../../../externals/QuaZIP/src
	${PROJECT_SOURCE_DIR}/../../../externals/QuaZIP/src/zlib
	${PROJECT_SOURCE_DIR}/../../../externals/CCM/src
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt5Network_INCLUDE_DIRS}
	${Qt5Svg_INCLUDE_DIRS}
	${Qt5PrintSupport_INCLUDE_DIRS}
)

# collect a list of all source files
file( GLOB APP_SRCS 		${PROJECT_SOURCE_DIR}/../../src/*.cpp )
file( GLOB APP_ACTION_SRCS 	${PROJECT_SOURCE_DIR}/../../src/actions/*.cpp )
file( GLOB APP_3D_SRCS 		${PROJECT_SOURCE_DIR}/../../src/core3D/*.cpp )

# collect a list of all header files (to be used in MOC compiler)
file( GLOB APP_HDRS 		${PROJECT_SOURCE_DIR}/../../src/*.h )
file( GLOB APP_ACTION_HDRS 	${PROJECT_SOURCE_DIR}/../../src/action/*.h )
file( GLOB APP_3D_HDRS 		${PROJECT_SOURCE_DIR}/../../src/core3D/*.h )

set( APP_SRCS 
	${APP_SRCS} 
	${APP_ACTION_SRCS} 
	${APP_3D_SRCS} 
)
set( APP_HDRS 
	${APP_HDRS} 
	${APP_ACTION_HDRS} 
	${APP_3D_HDRS} 
)

# collect a list of all form files
file( GLOB APP_UIS ${PROJECT_SOURCE_DIR}/../../src/*.ui )

# The Qt resource file (*.qrc)
set( APP_QRCS	
	${PROJECT_SOURCE_DIR}/../../resources/SIM-VICUS.qrc 
	${PROJECT_SOURCE_DIR}/../../resources/qdarkstyle/style.qrc
	${PROJECT_SOURCE_DIR}/../../src/shaders/shaders.qrc 
)

# set rc file
set ( APP_WIN_RC
	${PROJECT_SOURCE_DIR}/../../resources/SIM-VICUS.rc
)

# run uic and moc compilers, add resources

qt5_add_resources( APP_RC_SRCS ${APP_QRCS} )
qt5_wrap_ui( APP_UI_SRCS ${APP_UIS} )
qt5_wrap_cpp( APP_MOC_SRCS ${APP_HDRS} )

# generate list of all required QT_LIBRARIES
set ( QT_LIBRARIES 
	Qt5::Widgets
	Qt5::PrintSupport
	Qt5::Svg
	Qt5::Xml
	Qt5::Network
	Qt5::Concurrent
)

# build application executable for the different platforms
if( WIN32 )
	add_executable( ${PROJECT_NAME} WIN32 
					 ${APP_SRCS} ${APP_MOC_SRCS} ${APP_RC_SRCS} ${APP_UI_SRCS} ${APP_WIN_RC}
	)

	set(WIN_FRAMEWORKS iphlpapi gdiplus gdi32 user32 ole32)
	
	# enable console window (to see debug/profiler messages)
	set_target_properties( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE" )
	
endif( WIN32 ) 

if( UNIX )
	if( APPLE )
		add_executable( ${PROJECT_NAME} MACOSX_BUNDLE 
						${APP_SRCS} ${APP_MOC_SRCS} ${APP_RC_SRCS} ${APP_UI_SRCS}
						${PROJECT_SOURCE_DIR}/../../resources/gfx/SIM-VICUS.icns
		)
	else( APPLE )
		add_executable( ${PROJECT_NAME} 
						${APP_SRCS} ${APP_MOC_SRCS} ${APP_RC_SRCS} ${APP_UI_SRCS} 
						
		)
	endif( APPLE ) 

endif( UNIX )


# link against the dependent libraries
target_link_libraries( ${PROJECT_NAME} 
	Vicus
	QtExt
	QuaZIP
	Nandrad
	CCM
	QtExt
	IBK
	TiCPP
	QuaZIP
	qwt6
	${QT_LIBRARIES}
	${OPENGL_LIBRARIES}
	${APPLE_FRAMEWORKS}
	${WIN_FRAMEWORKS}
)






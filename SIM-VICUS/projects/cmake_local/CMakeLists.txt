# CMakeLists.txt file for SIM-VICUS

project( SIM-VICUS )

# add include directories
include_directories(
	${PROJECT_SOURCE_DIR}/../../src			# needed so that ui-generated header files find our own headers
	${PROJECT_SOURCE_DIR}/../../../externals/IBK/src
	${PROJECT_SOURCE_DIR}/../../../externals/Nandrad/src
	${PROJECT_SOURCE_DIR}/../../../externals/zlib/src
	${PROJECT_SOURCE_DIR}/../../../externals/minizip/src
	${PROJECT_SOURCE_DIR}/../../../externals/TiCPP/src
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt5Network_INCLUDE_DIRS}
	${Qt5Svg_INCLUDE_DIRS}
	${Qt5PrintSupport_INCLUDE_DIRS}
)

# collect a list of all source files
file( GLOB APP_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )

# collect a list of all header files (to be used in MOC compiler)
file( GLOB APP_HDRS ${PROJECT_SOURCE_DIR}/../../src/*.h )

# collect a list of all form files
file( GLOB APP_UIS ${PROJECT_SOURCE_DIR}/../../src/*.ui )

# The Qt resource file (*.qrc)
set( APP_QRCS	${PROJECT_SOURCE_DIR}/../../resources/SIM-VICUS.qrc )

# set rc file
set ( APP_WIN_RC
	${PROJECT_SOURCE_DIR}/../../resources/SIM-VICUS.rc
)

# run uic and moc compilers, add resources
if (Qt5Widgets_INCLUDE_DIRS)

	qt5_add_resources( APP_RC_SRCS ${APP_QRCS} )
	qt5_wrap_ui( APP_UI_SRCS ${APP_UIS} )
	qt5_wrap_cpp( APP_MOC_SRCS ${APP_HDRS} )

	# generate list of all required QT_LIBRARIES
	set ( QT_LIBRARIES 
		Qt5::Widgets
		Qt5::Svg
		Qt5::Network
		Qt5::PrintSupport
	)

else (Qt5Widgets_INCLUDE_DIRS)

	qt4_add_resources( APP_RC_SRCS ${APP_QRCS} )
	qt4_wrap_ui( APP_UI_SRCS ${APP_UIS} )
	qt4_wrap_cpp( APP_MOC_SRCS ${APP_HDRS} )

	# add missing QTMAIN library
	set( QT_LIBRARIES 
		${QT_LIBRARIES}
		${QT_QTMAIN_LIBRARY} # to avoid "error LNK2019: unresolved external symbol _WinMain@16 referenced in function ___tmainCRTStartup"
	)

endif (Qt5Widgets_INCLUDE_DIRS)


if( WIN32 )
	set (LINK_Z_LIBS zlibstatic)
else( WIN32 )
	set (LINK_Z_LIBS z dl)
endif( WIN32 )

# build application executable for the different platforms
if( WIN32 )
	add_executable( ${PROJECT_NAME} WIN32 
					 ${APP_SRCS} ${APP_MOC_SRCS} ${APP_RC_SRCS} ${APP_UI_SRCS} ${APP_WIN_RC}
	)

	set(WIN_FRAMEWORKS iphlpapi)

endif( WIN32 ) 

if( UNIX )
	if( APPLE )
		add_executable( ${PROJECT_NAME} MACOSX_BUNDLE 
						 ${APP_SRCS} ${APP_MOC_SRCS} ${APP_RC_SRCS} ${APP_UI_SRCS} 
		)
	else( APPLE )
		add_executable( ${PROJECT_NAME} 
						${APP_SRCS} ${APP_MOC_SRCS} ${APP_RC_SRCS} ${APP_UI_SRCS} 
						${PROJECT_SOURCE_DIR}/../../resources/SIM-VICUS.icns
		)
	endif( APPLE ) 

endif( UNIX )


# link against the dependent libraries
target_link_libraries( ${PROJECT_NAME} 
	Nandrad
	IBK
	TiCPP
	minizip
	${LINK_Z_LIBS}
	${QT_LIBRARIES}
	${APPLE_FRAMEWORKS}
	${WIN_FRAMEWORKS}
)






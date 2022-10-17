# CMakeLists.txt file for NandradFMUGenerator


project( NandradFMUGenerator )

# add include directories
include_directories(
	${PROJECT_SOURCE_DIR}/../../src			# needed so that ui-generated header files find our own headers
	${PROJECT_SOURCE_DIR}/../../../externals/IBK/src
	${PROJECT_SOURCE_DIR}/../../../externals/IBKMK/src
	${PROJECT_SOURCE_DIR}/../../../externals/Nandrad/src
	${PROJECT_SOURCE_DIR}/../../../externals/QtExt/src
	${PROJECT_SOURCE_DIR}/../../../externals/QuaZIP/src
	${PROJECT_SOURCE_DIR}/../../../externals/QuaZIP/src/zlib
	${Qt5Widgets_INCLUDE_DIRS}
)

# collect a list of all source files
file( GLOB APP_SRCS 		${PROJECT_SOURCE_DIR}/../../src/*.cpp )

# collect a list of all header files (to be used in MOC compiler)
file( GLOB APP_HDRS 		${PROJECT_SOURCE_DIR}/../../src/*.h )

# collect a list of all form files
file( GLOB APP_UIS ${PROJECT_SOURCE_DIR}/../../src/*.ui )

# add Qt-Keywordlist files
set( APP_SRCS
	${APP_SRCS}
	${PROJECT_SOURCE_DIR}/../../../externals/Nandrad/srcTranslations/NANDRAD_KeywordListQt.cpp
)

# The Qt resource file (*.qrc)
set( APP_QRCS	
	${PROJECT_SOURCE_DIR}/../../resources/NandradFMUGenerator.qrc 
)

# set rc file
set ( APP_WIN_RC
	${PROJECT_SOURCE_DIR}/../../resources/NandradFMUGenerator.rc
)

# run uic and moc compilers, add resources

qt5_add_resources( APP_RC_SRCS ${APP_QRCS} )
qt5_wrap_ui( APP_UI_SRCS ${APP_UIS} )
qt5_wrap_cpp( APP_MOC_SRCS ${APP_HDRS} )

# Note: this is just so that the manually build app bundle is basically configured when testing in 
#       MACOS and has a decent icon. In deployment mode, the same PList file needs to be used.
if( APPLE )
  set( GUI_TYPE MACOSX_BUNDLE )
  set( MACOSX_BUNDLE_BUNDLE_NAME NANDRADFMUGenerator )
  set( MACOSX_BUNDLE_BUNDLE_VERSION 1.0 )
  set( MACOSX_BUNDLE_COPYRIGHT "SIM-VICUS Team 2020-today, GPL Licensed" )
  set( MACOSX_BUNDLE_GUI_IDENTIFIER com.IBK.NANDRADFMUGenerator )
  set( MACOSX_BUNDLE_INFO_STRING "NANDRAD FMU Generator" )
  set( MACOSX_BUNDLE_LONG_VERSION_STRING "Version 1.0.0" )
  set( MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0" ) # the version shown in Finder
  set( MACOSX_BUNDLE_ICON_FILE NandradFMUGenerator.icns )
  SET_SOURCE_FILES_PROPERTIES( ${PROJECT_SOURCE_DIR}/../../resources/logo/NandradFMUGenerator.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif( APPLE )


# generate list of all required QT_LIBRARIES
set ( QT_LIBRARIES 
	Qt5::Widgets
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
						${PROJECT_SOURCE_DIR}/../../resources/logo/NandradFMUGenerator.icns
		)
	else( APPLE )
		add_executable( ${PROJECT_NAME} 
						${APP_SRCS} ${APP_MOC_SRCS} ${APP_RC_SRCS} ${APP_UI_SRCS} 
						
		)
	endif( APPLE ) 

endif( UNIX )


# link against the dependent libraries
target_link_libraries( ${PROJECT_NAME} 
	QtExt
	QuaZIP
	Nandrad
	IBKMK
	IBK
	TiCPP
	${QT_LIBRARIES}
	${APPLE_FRAMEWORKS}
	${WIN_FRAMEWORKS}
)


if (UNIX AND NOT APPLE)

	# installation targets for Unix systems
	include(GNUInstallDirs)
	
	# executable -> /usr/bin
	install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin )
	
	# Man-page
	install(FILES ${PROJECT_SOURCE_DIR}/../../doc/${PROJECT_NAME}.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 )

	# Desktop file
	install(FILES ${PROJECT_SOURCE_DIR}/../../resources/nandradfmugenerator.desktop 
			DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications )

	# Icons
	set(ICON_ROOT_DIR ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor)

	foreach(ICON_SIZE 256 128 64 32)
		install(FILES ${PROJECT_SOURCE_DIR}/../../resources/logo/icons/NandradIcon_${ICON_SIZE}.png 
				DESTINATION ${ICON_ROOT_DIR}/${ICON_SIZE}x${ICON_SIZE}/apps 
				RENAME nandrad.png)
	endforeach()

endif (UNIX AND NOT APPLE)

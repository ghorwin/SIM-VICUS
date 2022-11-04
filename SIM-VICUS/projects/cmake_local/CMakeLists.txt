# CMakeLists.txt file for SIM-VICUS

project( SIM-VICUS )

if (COMMAND cmake_policy)
  # set linker search policy new
  cmake_policy(SET CMP0003 NEW)
  # Policy CMP0020: Automatically link Qt executables to qtmain target on Windows.
  cmake_policy(SET CMP0020 NEW)
endif (COMMAND cmake_policy)

# add include directories
include_directories(
	${PROJECT_SOURCE_DIR}/../../src			# needed so that ui-generated header files find our own headers
	${PROJECT_SOURCE_DIR}/../../src/actions
	${PROJECT_SOURCE_DIR}/../../src/core3D
	${PROJECT_SOURCE_DIR}/../../../externals/IBK/src
	${PROJECT_SOURCE_DIR}/../../../externals/IBKMK/src
	${PROJECT_SOURCE_DIR}/../../../externals/Nandrad/src
	${PROJECT_SOURCE_DIR}/../../../externals/Nandrad/srcTranslations
	${PROJECT_SOURCE_DIR}/../../../externals/TiCPP/src
	${PROJECT_SOURCE_DIR}/../../../externals/Vicus/src
	${PROJECT_SOURCE_DIR}/../../../externals/Vicus/srcTranslations
	${PROJECT_SOURCE_DIR}/../../../externals/QtExt/src
	${PROJECT_SOURCE_DIR}/../../../externals/qwt/src
	${PROJECT_SOURCE_DIR}/../../../externals/QuaZIP/src
	${PROJECT_SOURCE_DIR}/../../../externals/QuaZIP/src/zlib
	${PROJECT_SOURCE_DIR}/../../../externals/CCM/src
	${PROJECT_SOURCE_DIR}/../../../externals/Shading/src
	${PROJECT_SOURCE_DIR}/../../../externals/IDFReader/src
	${PROJECT_SOURCE_DIR}/../../../externals/clipper/src
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

# add Qt-Keywordlist files
set( APP_SRCS
	${APP_SRCS}
	${PROJECT_SOURCE_DIR}/../../../externals/Nandrad/srcTranslations/NANDRAD_KeywordListQt.cpp
	${PROJECT_SOURCE_DIR}/../../../externals/Vicus/srcTranslations/VICUS_KeywordListQt.cpp
)

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
	# Mind: when commenting out next line, you need to link against WinMain
	#set_target_properties( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE" )
	
endif( WIN32 ) 

# Note: this is just so that the manually build app bundle is basically configured when testing in 
#       MACOS and has a decent icon. In deployment mode, the same PList file needs to be used.
if( APPLE )
  set( GUI_TYPE MACOSX_BUNDLE )
  set( MACOSX_BUNDLE_BUNDLE_NAME SIM-VICUS )
  set( MACOSX_BUNDLE_BUNDLE_VERSION 0.6 )
  set( MACOSX_BUNDLE_COPYRIGHT "SIM-VICUS Team 2020-today, GPL Licensed" )
  set( MACOSX_BUNDLE_GUI_IDENTIFIER com.IBK.SIM-VICUS )
  set( MACOSX_BUNDLE_INFO_STRING "SIM-VICUS 0.6" )
  set( MACOSX_BUNDLE_LONG_VERSION_STRING "Version 0.6.1" )
  set( MACOSX_BUNDLE_SHORT_VERSION_STRING "0.6" ) # the version shown in Finder
  set( MACOSX_BUNDLE_ICON_FILE SIM-VICUS.icns )
  SET_SOURCE_FILES_PROPERTIES( ${PROJECT_SOURCE_DIR}/../../resources/gfx/SIM-VICUS.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif( APPLE )

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
	Shading
	DataIO
	CCM
	QtExt
	IDFReader
	IBKMK
	clipper
	IBK
	TiCPP
	QuaZIP
	qwt6
	${QT_LIBRARIES}
	${OPENGL_LIBRARIES}
	${APPLE_FRAMEWORKS}
	${WIN_FRAMEWORKS}
)


# Support for 'make install' on Unix/Linux (not on MacOS!)
if (UNIX AND NOT APPLE)

	# installation targets for Unix systems
	include(GNUInstallDirs)

	# executable -> /usr/bin
	install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin )

	# Man-page
	install(FILES ${PROJECT_SOURCE_DIR}/../../doc/${PROJECT_NAME}.1 
			DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 )

	# Translation files
	install(FILES ${PROJECT_SOURCE_DIR}/../../resources/translations/${PROJECT_NAME}_de.qm 
			DESTINATION ${CMAKE_INSTALL_LOCALEDIR}/de/LC_MESSAGES/
			RENAME ${PROJECT_NAME}.qm)

	# Desktop file
	install(FILES ${PROJECT_SOURCE_DIR}/../../resources/simvicus.desktop 
			DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications )

	# Mime type
	install(FILES ${PROJECT_SOURCE_DIR}/../../resources/simvicus.xml 
			DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/mime/packages )

	# Icons
	set(ICON_ROOT_DIR ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor)

	foreach(ICON_SIZE 512 256 128 96 64 48 32 24 16)
		install(FILES ${PROJECT_SOURCE_DIR}/../../resources/logo/icons/Icon_${ICON_SIZE}.png 
				DESTINATION ${ICON_ROOT_DIR}/${ICON_SIZE}x${ICON_SIZE}/apps 
				RENAME simvicus.png)

		install(FILES ${PROJECT_SOURCE_DIR}/../../resources/logo/icons/Icon_${ICON_SIZE}.png 
				DESTINATION ${ICON_ROOT_DIR}/${ICON_SIZE}x${ICON_SIZE}/mimetypes 
				RENAME application-simvicus.png)
	endforeach()

	# Databases
	file( GLOB DB_FILES ${PROJECT_SOURCE_DIR}/../../../data/*.xml )
	install(FILES  ${DB_FILES}
			DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/simvicus)

endif (UNIX AND NOT APPLE)



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
						${PROJECT_SOURCE_DIR}/../../resources/NandradFMUGenerator.icns
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






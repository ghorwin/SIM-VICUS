###########################################################
#
# Qt project configuration for FMUs
#
# Include this file *after* you have set "template=lib"!
###########################################################

# load the central configuration file for all IBK dependent libraries
!include( ../../externals/IBK/projects/Qt/CONFIG.pri ) {
	message( "No custom build options specified" )
}

CONFIG			+= silent

CONFIG(release, debug|release) {
#	message( "Setting NDEBUG define" )
	DEFINES += NDEBUG
}

linux-g++ | linux-g++-64 | macx {

	# our code doesn't check errno after calling math functions
	# so it is perfectly safe to disable it in favor of better performance
	# use *= to uniquely assign option
	QMAKE_CXXFLAGS   *= -fno-math-errno

	# create "Position Independent Code"
	QMAKE_CXXFLAGS   *= -fPIC
}

contains( OPTIONS, sanitize_checks ) {

	CONFIG(debug, debug|release) {
		CONFIG += sanitizer
		CONFIG += sanitize_address
		CONFIG += sanitize_undefined
	}

	linux-g++ | linux-g++-64 | macx {
		QMAKE_CXXFLAGS_DEBUG   *= -fsanitize=address -fno-omit-frame-pointer
	}
}

# check if 32 or 64 bit version and set prefix variable for using in output paths
greaterThan(QT_MAJOR_VERSION, 4) {
	contains(QT_ARCH, i386): {
		DIR_PREFIX =
	} else {
		DIR_PREFIX = _x64
	}
} else {
	DIR_PREFIX =
}

#
# *** Libraries/FMUs ***
#
# This section contains all fmus specific settings. It can be enabled
# by setting 'lib' the TEMPLATE environment variable.
# It defines DESTDIR, OBJECTS_DIR, DLLDESTDIR and sets shared in CONFIG
# variable to all libraries.

equals(TEMPLATE,lib) {

	QT -=	core gui

	CONFIG += warn_on
	# fmus are shared libs
	CONFIG += shared

	# we use c++11
	CONFIG += c++11

	# set this even in case of no Qt library compilation to get mocs/uis organized in subdirs
	MOC_DIR = moc
	UI_DIR = ui

	CONFIG(debug, debug|release) {
		OBJECTS_DIR = debug$${DIR_PREFIX}
		DESTDIR = ../../../../bin/debug$${DIR_PREFIX}
		DLLDESTDIR = ../../../../bin/debug$${DIR_PREFIX}
	}
	else {
		OBJECTS_DIR = release$${DIR_PREFIX}
		DESTDIR = ../../../../bin/release$${DIR_PREFIX}
		DLLDESTDIR = ../../../../bin/release$${DIR_PREFIX}
	}

	QMAKE_CXXFLAGS += -std=c++11

	# using of shared libs only for non MC compiler
	# MS compiler needs explicite export statements in case of shared libs
	win32-msvc* {
		DEFINES += NOMINMAX
		DEFINES += _CRT_SECURE_NO_WARNINGS
		CONFIG(debug, debug|release) {
			QMAKE_CXXFLAGS += /GS /RTC1
		}
	}

	# we need to link against our libs
	QMAKE_LIBDIR += ../../../../externals/lib$${DIR_PREFIX}
	LIBS += -L../../../../externals/lib$${DIR_PREFIX}

	win32:LIBS += -lshell32
	win32:LIBS += -liphlpapi
}

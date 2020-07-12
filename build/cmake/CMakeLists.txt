# CMakeLists.txt file for SIM-VICUS and dependent libraries

# The project name
project( SIM-VICUS )

# Require a fairly recent cmake version
cmake_minimum_required( VERSION 2.8.12 )

# -------------------------------------------------------------
# MACRO definitions
# -------------------------------------------------------------

# Macros to hide/show cached variables.
# These two macros can be used to "hide" or "show" in the
# list of cached variables various variables and/or options
# that depend on other options.
# Note that once a variable is modified, it will preserve its
# value (hidding it merely makes it internal)

MACRO(HIDE_VARIABLE var)
  IF(DEFINED ${var})
SET(${var} "${${var}}" CACHE INTERNAL "")
  ENDIF(DEFINED ${var})
ENDMACRO(HIDE_VARIABLE)

MACRO(SHOW_VARIABLE var type doc default)
  IF(DEFINED ${var})
SET(${var} "${${var}}" CACHE "${type}" "${doc}" FORCE)
  ELSE(DEFINED ${var})
SET(${var} "${default}" CACHE "${type}" "${doc}")
  ENDIF(DEFINED ${var})
ENDMACRO(SHOW_VARIABLE)

# -------------------------------------------------------------
# Initial commands
# -------------------------------------------------------------

# Uncomment this to enable detailed make output
#set( CMAKE_VERBOSE_MAKEFILE ON )

# Hide some more cache variables to keep things tidy
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)
MARK_AS_ADVANCED(EXECUTABLE_OUTPUT_PATH LIBRARY_OUTPUT_PATH)

# Set default build type
if (NOT CMAKE_BUILD_TYPE)
	set( CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
		"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif (NOT CMAKE_BUILD_TYPE)

# on Unix we want really detailed warnings
if (UNIX)
	ADD_DEFINITIONS( -Wall -fPIC)
endif (UNIX)

# add VC compiler-specific flags (NONMINMAX define and disable "unsafe" warnings)
if (MSVC)
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D\"NOMINMAX\" /wd4996" )
endif (MSVC)

# -------------------------------------------------------------
# Packages
# -------------------------------------------------------------

# automatically add CMAKE_CURRENT_SOURCE_DIR and CMAKE_CURRENT_BINARY_DIR to the include directories in every processed CMakeLists.txt
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Concurrent REQUIRED)

message("*** Building with Qt5, Version ${Qt5Widgets_VERSION} ***")

# when building with gcc/icc add compile flag
if (NOT MSVC)
	if (APPLE)
	  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++" )
	else (APPLE)
	  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -std=gnu++11" )
	endif (APPLE)
endif (NOT MSVC)


if( APPLE )
	find_library(IOKIT NAMES IOKit REQUIRED)
	find_library(COREFOUNDATION NAMES CoreFoundation REQUIRED)
	find_library(SECURITY_FRAMEWORK Security REQUIRED)
	find_library(SYSTEM_CONFIGURATION SystemConfiguration REQUIRED)

	set(APPLE_FRAMEWORKS
		${IOKIT}
		${SECURITY_FRAMEWORK}
		${COREFOUNDATION}
		${SYSTEM_CONFIGURATION}
	)
endif( APPLE )

# -------------------------------------------------------------
# Tools of the build chain
# -------------------------------------------------------------

# KeywordListCreator
add_subdirectory( ../../NandradCodeGenerator/projects/cmake_local NandradCodeGenerator )

# -------------------------------------------------------------
# subdirectories and subprojects
# -------------------------------------------------------------

# add subdirectories for all builds
add_subdirectory( ../../externals/CCM/projects/cmake_local CCM )
add_subdirectory( ../../externals/DataIO/projects/cmake_local DataIO )
add_subdirectory( ../../externals/IBK/projects/cmake_local IBK )
add_subdirectory( ../../externals/SuiteSparse/projects/cmake_local SuiteSparse )
add_subdirectory( ../../externals/Nandrad/projects/cmake_local Nandrad )

#add_subdirectory( ../../externals/zlib/projects/cmake_local zlib )
#add_subdirectory( ../../externals/qwt/projects/cmake_local qwt )
#add_subdirectory( ../../externals/QtExt/projects/cmake_local QtExt )
#set(SCICHART_HAS_EMFENGINE ON CACHE BOOL "Build SciChart with EMFEngine" FORCE)
#add_subdirectory( ../../externals/SciChart/projects/cmake_local SciChart )
add_subdirectory( ../../externals/TiCPP/projects/cmake_local TiCPP )

if( WIN32 )
#	add_subdirectory( ../../externals/EmfEngine/projects/cmake_local EmfEngine )
endif( WIN32 )


# -------------------------------------------------------------
# main project
# -------------------------------------------------------------

#add_subdirectory( ../../SIM-VICUS/projects/cmake_local SIM-VICUS)
#add_subdirectory( ../../CCD2C6BConverter/projects/cmake_local CCD2C6BConverter)



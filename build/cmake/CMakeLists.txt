# CMakeLists.txt file for SIM-VICUS and dependent libraries

# The project name
project( SIM-VICUS )

# Require a fairly recent cmake version
cmake_minimum_required( VERSION 2.8...3.10 )

# enable @rpath on MacOS
cmake_policy(SET CMP0042 NEW)

if (${CMAKE_VERSION} VERSION_GREATER "3.11") 
  # use new OpenGL libs when having both new and legacy OpenGL libs on Linux
  cmake_policy(SET CMP0072 NEW) 
endif()

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

if (MSVC)

	# add VC compiler-specific flags (NONMINMAX define and disable "unsafe" warnings)
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D\"NOMINMAX\" /wd4996" )

else (MSVC)

	# on Unix we want really detailed warnings
	ADD_DEFINITIONS( -Wall -fPIC)
	# when building with gcc/icc add compile flag
	if (APPLE)
		#message( STATUS "Mac compiler ID = ${CMAKE_C_COMPILER_ID}" )
		if ( CMAKE_C_COMPILER_ID MATCHES "GNU")
			set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
		else ( CMAKE_C_COMPILER_ID MATCHES "GNU")
			set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++" )
		endif ( CMAKE_C_COMPILER_ID MATCHES "GNU")
	else (APPLE)
		set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -std=gnu++11" )
	endif (APPLE)

endif (MSVC)


# -------------------------------------------------------------
# Packages
# -------------------------------------------------------------

# automatically add CMAKE_CURRENT_SOURCE_DIR and CMAKE_CURRENT_BINARY_DIR to the include directories in every processed CMakeLists.txt
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (DISABLE_QT)
	message("*** Building without Qt5 ***")
else (DISABLE_QT)
	find_package(Qt5Widgets REQUIRED)
	find_package(Qt5Xml REQUIRED)
	find_package(Qt5Svg REQUIRED)
	find_package(Qt5PrintSupport REQUIRED)
	find_package(Qt5Network REQUIRED)
	find_package(Qt5Concurrent REQUIRED)

	message("*** Building with Qt5, Version ${Qt5Widgets_VERSION} ***")
	
	# we need OpenGL
	find_package( OpenGL REQUIRED )

endif (DISABLE_QT)

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
# Support for build flags
# -------------------------------------------------------------
include( ../../externals/IBK/projects/cmake_local/cmake_pri/CMakeLists.txt  )

# -------------------------------------------------------------
# Which modules to build?
# -------------------------------------------------------------

option( BUILD_SUNDIALS "Compile Sundials library" ON )
# hide the options of the sundials lib that we don't need
MARK_AS_ADVANCED(BUILD_ARKODE)
SET(BUILD_ARKODE OFF CACHE BOOL "ARKODE off" FORCE)
MARK_AS_ADVANCED(BUILD_CVODE)
MARK_AS_ADVANCED(BUILD_CVODES)
set(BUILD_CVODES OFF CACHE BOOL "CVODES off" FORCE)
MARK_AS_ADVANCED(BUILD_IDA)
MARK_AS_ADVANCED(BUILD_IDAS)
set(BUILD_IDA OFF CACHE BOOL "IDA off" FORCE)
set(BUILD_IDAS OFF CACHE BOOL "IDAS off" FORCE)
MARK_AS_ADVANCED(BUILD_KINSOL)
MARK_AS_ADVANCED(BUILD_FCMIX)
set(BUILD_FCMIX OFF CACHE BOOL "FCMIX off" FORCE)
MARK_AS_ADVANCED(BUILD_SHARED_LIBS)
MARK_AS_ADVANCED(PRECISION)
MARK_AS_ADVANCED(EXAMPLES_ENABLE)
set(EXAMPLES_ENABLE OFF CACHE BOOL "EXAMPLES off" FORCE)

# activate options for KLU solver
set(KLU_ENABLE ON CACHE BOOL "KLU on" FORCE)
set(KLU_INCLUDE_DIR ../../../externals/SuiteSparse/src/include)

# -------------------------------------------------------------
# Tools of the build chain
# -------------------------------------------------------------

# NandradCodeGenerator
add_subdirectory( ../../NandradCodeGenerator/projects/cmake_local NandradCodeGenerator )

# -------------------------------------------------------------
# subdirectories and subprojects
# -------------------------------------------------------------

# add subdirectories for all builds
add_subdirectory( ../../externals/sundials/src sundials)
add_subdirectory( ../../externals/SuiteSparse/projects/cmake_local SuiteSparse )
add_subdirectory( ../../externals/DataIO/projects/cmake_local DataIO )
add_subdirectory( ../../externals/CCM/projects/cmake_local CCM )
add_subdirectory( ../../externals/IBK/projects/cmake_local IBK )
add_subdirectory( ../../externals/IBKMK/projects/cmake_local IBKMK )
add_subdirectory( ../../externals/Nandrad/projects/cmake_local Nandrad )
add_subdirectory( ../../externals/IntegratorFramework/projects/cmake_local IntegratorFramework )
add_subdirectory( ../../externals/Zeppelin/projects/cmake_local Zeppelin )
add_subdirectory( ../../externals/TiCPP/projects/cmake_local TiCPP )
add_subdirectory( ../../externals/clipper/projects/cmake_local clipper )
#add_subdirectory( ../../externals/zlib/src zlib )
add_subdirectory( ../../externals/IDFReader/projects/cmake_local IDFReader )

if (NOT DISABLE_QT)
	message("*** Building Qt-based libs ***")

	add_subdirectory( ../../externals/Shading/projects/cmake_local SHADING )
	add_subdirectory( ../../externals/RoomClipper/projects/cmake_local RoomClipper )
	add_subdirectory( ../../externals/Vicus/projects/cmake_local Vicus )
	add_subdirectory( ../../externals/qwt/projects/cmake_local qwt )
	add_subdirectory( ../../externals/QtExt/projects/cmake_local QtExt )
	add_subdirectory( ../../externals/QuaZIP/projects/cmake_local QuaZIP )

endif (NOT DISABLE_QT)



# -------------------------------------------------------------
# applications and FMI Interface
# -------------------------------------------------------------

add_subdirectory( ../../View3D/projects/cmake_local View3D)

add_subdirectory( ../../NandradSolver/projects/cmake_local NandradSolver)
add_subdirectory( ../../NandradSolverFMI/projects/cmake_local NandradSolverFMI)

if (NOT DISABLE_QT)
	add_subdirectory( ../../SIM-VICUS/projects/cmake_local SIM-VICUS)
	add_subdirectory( ../../NandradFMUGenerator/projects/cmake_local NandradFMUGenerator)
endif (NOT DISABLE_QT)


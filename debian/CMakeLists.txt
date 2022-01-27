# CMakeLists.txt file for SIM-VICUS debian package creation
# Author: Andreas Nicolai (andreas.nicolai -at- gmx.net)

# The project name
project( SimVicusDebPackage )

# Require a fairly recent cmake version
cmake_minimum_required( VERSION 3.10 )

# enable @rpath on MacOS
cmake_policy(SET CMP0042 NEW)

# Debian package build flag for IBK-based applications
add_definitions( -DIBK_BUILDING_DEBIAN_PACKAGE )

# Configure build
set( CMAKE_BUILD_TYPE Release CACHE STRING
	"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)

set( USE_OMP ON CACHE BOOL
	"Enable/Disable OpenMP build." FORCE)

add_subdirectory( build/cmake SimVicusPackage)

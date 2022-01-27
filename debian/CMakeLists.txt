# CMakeLists.txt file for SIM-VICUS debian package creation
# Author: Andreas Nicolai (andreas.nicolai -at- gmx.net)

# The project name
project( SimVicusDebPackage )

# Require a fairly recent cmake version
cmake_minimum_required( VERSION 3.10 )

# enable @rpath on MacOS
cmake_policy(SET CMP0042 NEW)

# Debian package build flag for IBK-based applications
if (UNIX AND NOT APPLE)
	ADD_DEFINITIONS( -DIBK_BUILDING_DEBIAN_PACKAGE )
endif (UNIX AND NOT APPLE)

add_subdirectory( build/cmake SimVicusPackage)

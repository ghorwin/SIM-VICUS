# CMakeLists.txt file for NandradSolver FMI v2

project( NandradSolverFMI )

if (CMAKE_COMPILER_IS_GNUCC)
	if (NOT APPLE)
		# link solver statically
		set( CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++" )
	endif (NOT APPLE)
endif (CMAKE_COMPILER_IS_GNUCC)

# add include directories
include_directories(
	${PROJECT_BINARY_DIR}/../sundials/include
	${PROJECT_SOURCE_DIR}/../../src
	${PROJECT_SOURCE_DIR}/../../../NandradSolver/src
	${PROJECT_SOURCE_DIR}/../../../externals/Nandrad/src
	${PROJECT_SOURCE_DIR}/../../../externals/IntegratorFramework/src
	${PROJECT_SOURCE_DIR}/../../../externals/CCM/src
	${PROJECT_SOURCE_DIR}/../../../externals/DataIO/src
	${PROJECT_SOURCE_DIR}/../../../externals/TiCPP/src
	${PROJECT_SOURCE_DIR}/../../../externals/IBK/src
	${PROJECT_SOURCE_DIR}/../../../externals/IBKMK/src
	${PROJECT_SOURCE_DIR}/../../../externals/Zeppelin/src
	${PROJECT_SOURCE_DIR}/../../../externals/sundials/src/include
	${PROJECT_SOURCE_DIR}/../../../externals/SuiteSparse/src/include
)

# link against the dependent libraries
set( LINK_LIBS
	NandradModel
	IntegratorFramework 
	Nandrad 
	DataIO
	IBKMK
	CCM
	IBK
	TiCPP
	Zeppelin
	sundials_cvode_static
	sundials_kinsol_static
	sundials_nvecserial_static
	SuiteSparse
)

# create shared library
add_library( ${PROJECT_NAME} SHARED
	${PROJECT_SOURCE_DIR}/../../src/InstanceData.cpp
	${PROJECT_SOURCE_DIR}/../../src/NandradModelFMU.cpp
	${PROJECT_SOURCE_DIR}/../../src/fmi2common/fmi2Functions.cpp
	${PROJECT_SOURCE_DIR}/../../src/fmi2common/InstanceDataCommon.cpp
)

# link against the dependent libraries
target_link_libraries( ${PROJECT_NAME} 
	${LINK_LIBS}
)

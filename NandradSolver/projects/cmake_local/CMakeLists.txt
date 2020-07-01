# CMakeLists.txt file for NandradSolver

project( NandradSolver )

if (CMAKE_COMPILER_IS_GNUCC)
	if (NOT APPLE)
		# link solver statically
		set( CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++" )
	endif (NOT APPLE)
endif (CMAKE_COMPILER_IS_GNUCC)

# add include directories
include_directories(
	${PROJECT_SOURCE_DIR}/../../src
	${PROJECT_SOURCE_DIR}/../../../externals/IBK/src
	${PROJECT_SOURCE_DIR}/../../../externals/IBKMK/src
	${PROJECT_SOURCE_DIR}/../../../externals/Nandrad/src
	${PROJECT_SOURCE_DIR}/../../../externals/NandradModel/src
	${PROJECT_SOURCE_DIR}/../../../externals/IntegratorFramework/src
)

if (OPENMP_FOUND)
	set( SUNDIALS_OMP_LINK_LIBS sundials_nvecopenmp_static )
endif (OPENMP_FOUND)

# set variable for dependent libraries
set( LINK_LIBS
	NandradModel
	IntegratorFramework 
	Nandrad 
	DelphinLight
	Materials
	WallModel
	WindowModel
	IBKMK
	DataIO
	CCM
	IBK
	TiCPP
	Zeppelin
	sundials_cvode_static
	sundials_kinsol_static
	sundials_nvecserial_static
	${SUNDIALS_OMP_LINK_LIBS}
	SuiteSparse
)

if( WIN32 )
	set (LINK_LIBS ${LINK_LIBS} zlibstatic)
else( WIN32 )
	set (LINK_LIBS ${LINK_LIBS} zlib dl)
endif( WIN32 )

add_executable( ${PROJECT_NAME}
	${PROJECT_SOURCE_DIR}/../../src/main.cpp
	${PROJECT_SOURCE_DIR}/../../resources/resource.h
	${PROJECT_SOURCE_DIR}/../../resources/NandradSolver.rc
)

# link against the dependent libraries
target_link_libraries( ${PROJECT_NAME} 
	${LINK_LIBS}
)




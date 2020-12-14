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
	${CMAKE_BINARY_DIR}/sundials/include
	${PROJECT_SOURCE_DIR}/../../src
	${PROJECT_SOURCE_DIR}/../../../externals/sundials/src/include
	${PROJECT_SOURCE_DIR}/../../../externals/SuiteSparse/src/include
	${PROJECT_SOURCE_DIR}/../../../externals/CCM/src
	${PROJECT_SOURCE_DIR}/../../../externals/IBK/src
	${PROJECT_SOURCE_DIR}/../../../externals/IBKMK/src
	${PROJECT_SOURCE_DIR}/../../../externals/Nandrad/src
	${PROJECT_SOURCE_DIR}/../../../externals/IntegratorFramework/src
	${PROJECT_SOURCE_DIR}/../../../externals/TiCPP/src
	${PROJECT_SOURCE_DIR}/../../../externals/Zeppelin/src
)

file( GLOB NandradSolver_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )

# add NANDRAD_KeywordList.cpp
set( NandradSolver_SRCS
	${NandradSolver_SRCS}
	${PROJECT_SOURCE_DIR}/../../src/NM_KeywordList.cpp
)

# collect a list of all header files of the Nandrad library
file( GLOB NandradModel_HDRS ${PROJECT_SOURCE_DIR}/../../src/*.h )

# run the KeywordListCreator tool whenever the header files have changed
# to update the NANDRAD_KeywordList.h and NANDRAD_KeywordList.cpp file
add_custom_command (
	OUTPUT   ${PROJECT_SOURCE_DIR}/../../src/NM_KeywordList.cpp
	DEPENDS  ${NandradModel_HDRS} NandradCodeGenerator
	COMMAND  NandradCodeGenerator
	ARGS     NANDRAD_MODEL ${PROJECT_SOURCE_DIR}/../../src 0 NM ncg
)

if (OPENMP_FOUND)
	set( SUNDIALS_OMP_LINK_LIBS sundials_nvecopenmp_static )
endif (OPENMP_FOUND)

# set variable for dependent libraries
set( LINK_LIBS
	IntegratorFramework 
	Nandrad 
	IBKMK
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

#if( WIN32 )
#	set (LINK_LIBS ${LINK_LIBS} zlibstatic)
#else( WIN32 )
#	set (LINK_LIBS ${LINK_LIBS} zlib dl)
#endif( WIN32 )

add_executable( ${PROJECT_NAME}
	${NandradSolver_SRCS}
)

# link against the dependent libraries
target_link_libraries( ${PROJECT_NAME} 
	${LINK_LIBS}
)




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
	${PROJECT_BINARY_DIR}/../sundials/include
	${PROJECT_SOURCE_DIR}/../../src
	${PROJECT_SOURCE_DIR}/../../../externals/Nandrad/src
	${PROJECT_SOURCE_DIR}/../../../externals/NandradModel/src
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

# gather all cpp files in NandradSolver directory
file( GLOB NandradSolver_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )

# add NANDRAD_KeywordList.cpp
set( NandradSolver_SRCS
	${NandradSolver_SRCS}
	${PROJECT_SOURCE_DIR}/../../src/NM_KeywordList.cpp
)

# collect a list of all header files of the Nandrad library
file( GLOB NandradModel_HDRS ${PROJECT_SOURCE_DIR}/../../src/*.h )

# run the KeywordListCreator tool whenever the header files have changed
# to update the NM_KeywordList.h and NM_KeywordList.cpp file
add_custom_command (
	OUTPUT   ${PROJECT_SOURCE_DIR}/../../src/NM_KeywordList.cpp
	DEPENDS  ${NandradModel_HDRS} NandradCodeGenerator
	COMMAND  NandradCodeGenerator
	ARGS     NANDRAD_MODEL ${PROJECT_SOURCE_DIR}/../../src 0 NM ncg
)


# remove the main.cpp from the list of source files
list(REMOVE_ITEM NandradSolver_SRCS ${PROJECT_SOURCE_DIR}/../../../NandradSolver/src/main.cpp)

# build the library NandradModel
add_library( NandradModel STATIC
	${NandradSolver_SRCS}
)


# set variable for dependent libraries
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

# now build the NandradSolver executable - this only requires compiling the main.cpp
add_executable( ${PROJECT_NAME}
	${PROJECT_SOURCE_DIR}/../../src/main.cpp
)

# and link it against the dependent libraries
target_link_libraries( ${PROJECT_NAME} 
	${LINK_LIBS}
)


if (UNIX AND NOT APPLE)

	# installation targets for Unix systems
	include(GNUInstallDirs)
	
	# executable -> /usr/bin
	install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin )
	
	# Man-page
	install(FILES ${PROJECT_SOURCE_DIR}/../../doc/${PROJECT_NAME}.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 )

endif (UNIX AND NOT APPLE)


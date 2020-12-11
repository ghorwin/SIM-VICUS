# CMakeLists.txt file for SuiteSparse library

project( SuiteSparse )

include_directories(
	${PROJECT_SOURCE_DIR}/../../src/include
)

# collect a list of all source files of the SuiteSparse library
file( GLOB SuiteSparse_CONF_SRCS   ${PROJECT_SOURCE_DIR}/../../src/SuiteSparse_config/*.c ) 
file( GLOB SuiteSparse_AMD_SRCS  ${PROJECT_SOURCE_DIR}/../../src/AMD/Source/*.c )
file( GLOB SuiteSparse_BTF_SRCS  ${PROJECT_SOURCE_DIR}/../../src/BTF/Source/*.c )
file( GLOB SuiteSparse_COLAMD_SRCS   ${PROJECT_SOURCE_DIR}/../../src/COLAMD/Source/*.c ) 
file( GLOB SuiteSparse_KLU_SRCS   ${PROJECT_SOURCE_DIR}/../../src/KLU/Source/*.c ) 

set( SuiteSparse_SRCS 
	${SuiteSparse_CONF_SRCS} 
	${SuiteSparse_AMD_SRCS} 
	${SuiteSparse_BTF_SRCS}
	${SuiteSparse_COLAMD_SRCS}
	${SuiteSparse_KLU_SRCS}
)

# build the library
add_library( SuiteSparse STATIC
	${SuiteSparse_SRCS}
)


# CMakeLists.txt file for IntegratorFramework library

project( IntegratorFramework )

# collect a list of all source files of the IntegratorFramework library
file( GLOB IntegratorFramework_LIB_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )

# add include directories, also include binary dir, because sundials_config.h is
# being generated there
include_directories(
	${CMAKE_BINARY_DIR}/sundials/include
	${PROJECT_SOURCE_DIR}/../../../sundials/src/include
	${PROJECT_SOURCE_DIR}/../../../IBK/src 
	${PROJECT_SOURCE_DIR}/../../../IBKMK/src 
)

# build the library
add_library( IntegratorFramework STATIC
	${IntegratorFramework_LIB_SRCS}
)


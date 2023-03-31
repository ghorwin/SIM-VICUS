cmake_minimum_required(VERSION 3.10)
project(libdxfrw)

# Set the C++ compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Woverloaded-virtual")

# Define the header and source files
file(GLOB LIBDXFRW_PUBLIC_HEADERS "../../src/*.h")
file(GLOB LIBDXFRW_PRIVATE_HEADERS "../../src/intern/*.h")
file(GLOB LIBDXFRW_SOURCES "../../src/*.cpp" "../../src/intern/*.cpp")

# Define the library and its properties
add_library(${PROJECT_NAME} STATIC ${LIBDXFRW_SOURCES} ${LIBDXFRW_PUBLIC_HEADERS} ${LIBDXFRW_PRIVATE_HEADERS})

# Main library
file(GLOB_RECURSE sources "${PROJECT_SOURCE_DIR}/src/[a-zA-Z]*.cpp")
file(GLOB_RECURSE public_headers "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/[a-zA-Z]*.h")
file(GLOB_RECURSE private_headers "${PROJECT_SOURCE_DIR}/src/[a-zA-Z]*.h")

set(library_sources
  ${sources}
  ${public_headers}
  ${private_headers}
)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/vendor/glad/include)
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/vendor/glfw/include)
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/vendor/glfw/deps)
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/vendor/libtess2/Include)

option( ${PROJECT_NAME}_STATIC_LIB "Make ${PROJECT_NAME} a static library" ON )

if (${PROJECT_NAME}_STATIC_LIB)
  add_library(${PROJECT_NAME} STATIC ${library_sources})
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-D${PROJECT_NAME}_STATIC_DEFINE")
else()
  set(CMAKE_MACOSX_RPATH 1)
  add_library(${PROJECT_NAME} SHARED ${library_sources})
  if(UNIX)
    set_target_properties(tess2 PROPERTIES COMPILE_FLAGS "-fPIC")
  endif()
endif()

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} glad glfw tess2)

if (${PROJECT_NAME}_COVERAGE)
  add_coverage(${PROJECT_NAME})
endif()

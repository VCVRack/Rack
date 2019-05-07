cmake_minimum_required(VERSION 3.7)

project(VCVRack)

set(CMAKE_CXX_STANDARD 14)

set(RACK_APP rack)

set(RACK_GIT_REV "UNKNOWN")

find_package(Git)
if(GIT_FOUND)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    OUTPUT_VARIABLE RACK_GIT_REV
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
else()
  message(WARNING "Git cmd not found")
endif()

set(RACK_BUILD_VERSION "1.dev.${RACK_GIT_REV}")
message( STATUS "build ${RACK_APP} version '${RACK_BUILD_VERSION}'")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS include/*.hpp)
source_group("Header Files" FILES ${HEADERS})

add_executable(${RACK_APP} ${SOURCES})

target_compile_definitions(${RACK_APP} PRIVATE VERSION=${RACK_BUILD_VERSION})

if (MSVC)
  target_compile_definitions(${RACK_APP} PRIVATE _CRT_SECURE_NO_WARNINGS _USE_MATH_DEFINES ARCH_WIN)
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
   target_compile_definitions(${RACK_APP} PRIVATE ARCH_LIN)
   set_target_properties(${RACK_APP} PROPERTIES LINK_FLAGS -rdynamic)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
   target_compile_definitions(${RACK_APP} PRIVATE ARCH_MAC)
   set_target_properties(${RACK_APP} PROPERTIES LINK_FLAGS -export_dynamic)
endif ()

target_include_directories(${RACK_APP} PRIVATE include)
conan_target_link_libraries(${RACK_APP})

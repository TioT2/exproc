cmake_minimum_required(VERSION 3.14)

project(exproc)

# add cmake-specific flag to disable 'C with C++ compiler' deprecation warning
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Wno-deprecated")
endif()

# set language
set(EP_LANGUAGE CXX)

# set preferred C++ standard
set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE source CONFIGURE_DEPENDS src/*.c)

set_source_files_properties(${source} PROPERTIES LANGUAGE ${EP_LANGUAGE})

add_executable(exproc ${source})
target_link_libraries(exproc m)

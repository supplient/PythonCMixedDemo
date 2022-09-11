# CMakeList.txt : CMake project for hallo1, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

add_compile_definitions(HALLO_EXPORTS)

# Add source to this project's executable.
add_library(hallo STATIC "hallo.cpp" "hallo.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET hallo PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.

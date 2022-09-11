# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("cexts")


# C++ dlls
add_subdirectory("hallo")
add_subdirectory("cuda_hallo")


# Python bind
add_subdirectory("pybind")

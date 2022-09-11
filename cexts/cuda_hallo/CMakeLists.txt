cmake_minimum_required (VERSION 3.24) # Just for CUDA_ARCHITECTURES's native
enable_language(CUDA)

add_compile_definitions(CUDA_HALLO_EXPORTS)
add_library(cuda_hallo STATIC "kernel.cu" "kernel.h")
set_property(TARGET cuda_hallo PROPERTY CUDA_SEPARABLE_COMPILATION ON)
set_property(TARGET cuda_hallo PROPERTY CUDA_ARCHITECTURES native) # Comment this if your cmake < 3.24


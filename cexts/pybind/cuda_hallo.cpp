#include <pybind11/pybind11.h>
namespace py = pybind11;


#include "kernel.h"
PYBIND11_MODULE(cuda_hallo, m) {
    m.doc() = "pybind11 example plugin"; // optional module docstring

    m.def("test", &CudaTestFunc, "A function that adds two numbers");
}
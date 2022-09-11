#include <pybind11/pybind11.h>
namespace py = pybind11;


#include "hallo.h"
PYBIND11_MODULE(hallo, m) {
    m.doc() = "pybind11 example plugin"; // optional module docstring

    m.def("test", &TestFunc, "A function that adds two numbers");
}
cmake_minimum_required(VERSION 3.4...3.18)

# Output
set(OUT_DIR ${PROJECT_SOURCE_DIR}/dll)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUT_DIR})

# Dependency: pybind11
add_subdirectory(pybind11)

# Workhorse
function(build_pybind MODULE_NAME)
	set(PY_MODULE_NAME py_${MODULE_NAME})
	set(PYSTUB_MODULE_NAME pystub_${MODULE_NAME})

	# Build pyd
	pybind11_add_module(${PY_MODULE_NAME} ${MODULE_NAME}.cpp)
	set_target_properties(${PY_MODULE_NAME} PROPERTIES OUTPUT_NAME ${MODULE_NAME})
	target_link_libraries(${PY_MODULE_NAME} PRIVATE ${MODULE_NAME})
	target_include_directories(${PY_MODULE_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/${MODULE_NAME})

	# Generate pyi(stub file)
	add_custom_command(
		OUTPUT ${OUT_DIR}/${MODULE_NAME}.pyi
		WORKING_DIRECTORY ${OUT_DIR}
		COMMAND stubgen -m ${MODULE_NAME} -o ${OUT_DIR}
		COMMENT "Generating stub file for `${MODULE_NAME}` ${OUT_DIR}/${MODULE_NAME}.pyi"
	)
	add_custom_target(${PYSTUB_MODULE_NAME} ALL DEPENDS ${OUT_DIR}/${MODULE_NAME}.pyi)
	add_dependencies(${PYSTUB_MODULE_NAME} ${PY_MODULE_NAME})
endfunction()

# Modules
build_pybind(hallo)

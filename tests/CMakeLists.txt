cmake_minimum_required (VERSION 3.2)
project(sjson-cpp_unit_tests)

set(CMAKE_CXX_STANDARD 11)

include_directories("${PROJECT_SOURCE_DIR}/../includes")
include_directories("${PROJECT_SOURCE_DIR}/../external/catch-1.9.6")

# Grab all of our source files
file(GLOB_RECURSE SJSON_CPP_UNIT_TEST_SOURCE_FILES LIST_DIRECTORIES false
	${PROJECT_SOURCE_DIR}/sources/*.h
	${PROJECT_SOURCE_DIR}/sources/*.cpp)

create_source_groups("${SJSON_CPP_UNIT_TEST_SOURCE_FILES}" ${PROJECT_SOURCE_DIR})

add_executable(sjson-cpp_unit_tests ${SJSON_CPP_UNIT_TEST_SOURCE_FILES})
add_test(NAME UNIT COMMAND sjson-cpp_unit_tests)

setup_default_compiler_flags(${PROJECT_NAME})

install(TARGETS sjson-cpp_unit_tests RUNTIME DESTINATION bin)

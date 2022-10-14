cmake_minimum_required(VERSION 3.10)

project(crudechess LANGUAGES CXX VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_options(-Wall -Wextra -pedantic -Werror)


set(CRUDECHESS_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")
set(CRUDECHESS_BINARY_DIR "${PROJECT_SOURCE_DIR}/bin")
set(CRUDECHESS_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(CRUDECHESS_LIBRARIES_DIR "${PROJECT_SOURCE_DIR}/lib")
set(CRUDECHESS_TEST_DIR "${PROJECT_SOURCE_DIR}/test")

option(CRUDECHESS_TEST "Build and run unit tests" OFF)
option(CRUDECHESS_DEBUG "Create executable with debug symbols and no optimisation" OFF)

if(CRUDECHESS_DEBUG)
    message(STATUS "Mode: debug")
    add_compile_options(-g)
else()
    add_compile_options(-O2)
endif()

add_subdirectory(src)
add_subdirectory(lib)

# add_subdirectory(test)

# add_custom_command(
#     OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/runTests
#     COMMAND test
#     DEPENDS 
# )

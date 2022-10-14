find_package(GTest REQUIRED)
target_include_directories(${GTEST_INCLUDE_DIRS})
target_include_directories("${CRUDECHESS_INCLUDE_DIR}")

add_executable(runTests fen.cc "${CRUDECHESS_SOURCE_DIR}/board/fen.cc")
target_link_libraries(runTests ${GTEST_LIBRARIES} pthread)
target_link_libraries(runTests crudelog)

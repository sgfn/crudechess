add_subdirectory(src)

file(GLOB BOARD_SRC CONFIGURE_DEPENDS "*.hh" "*.cc")

add_executable(crudechess_test test_main.cc)

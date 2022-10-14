set(PROCNAME "crudechess_board")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__FILENAME__='\"$(subst ${CMAKE_CURRENT_LIST_DIR}/,,$(abspath $<))\"'")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__MODULE__='\"${PROCNAME}\"'")

file(GLOB BOARD_SRC *.cc)

add_executable(crudechess_board ${BOARD_SRC})

target_link_libraries(crudechess_board PUBLIC crudelog)

target_include_directories(crudechess_board PUBLIC "${CRUDECHESS_INCLUDE_DIR}")

install(TARGETS crudechess_board DESTINATION "${CRUDECHESS_BINARY_DIR}")

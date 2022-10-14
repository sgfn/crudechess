add_library(crudelog log.cc)

target_include_directories(crudelog PUBLIC "${CRUDECHESS_INCLUDE_DIR}")

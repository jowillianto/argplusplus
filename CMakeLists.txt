add_library(argplusplus)

target_sources(argplusplus
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/parser.cpp 
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/parser.hpp
)

target_include_directories(argplusplus
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
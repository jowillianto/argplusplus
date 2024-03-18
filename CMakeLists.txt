cmake_minimum_required(VERSION 3.10.0)

project(ArgPlusPlus)

# Set C++ Standards
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Add Arg Plus Plus as part of the library
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

# Now Link the rest
# add_subdirectory(test-lib)

add_executable(tests ${CMAKE_CURRENT_LIST_DIR}/tests.cpp)
target_link_directories(tests
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/test-lib
)
target_link_libraries(tests
  PUBLIC 
    argplusplus
)
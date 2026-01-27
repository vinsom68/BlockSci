cmake_minimum_required(VERSION 3.5)
project(blocksci)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

set(BlockSci_VERSION 0.7.0)

include(GNUInstallDirs)
include(CTest)

add_subdirectory(external)

add_subdirectory(src)
add_subdirectory(tools)
add_subdirectory(benchmark)
add_subdirectory(test)
add_subdirectory(example)

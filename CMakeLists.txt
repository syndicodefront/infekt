# CMake build file for the GTK port
# not to be used on Windows

cmake_minimum_required (VERSION 2.6)
project (INFEKT)

option (OPTION_GTK "Building iNFEKT-GTK")

set (CMAKE_BUILD_TYPE "Release")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

add_subdirectory (src)

set (CPACK_SOURCE_IGNORE_FILES
  .svn
  /build-out
  /dependencies
  /tools
  /project
)

include (CPack)

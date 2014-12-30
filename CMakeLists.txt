cmake_minimum_required(VERSION 2.6)

       
set(APP_NAME CellMotus)
project (${APP_NAME})

option(DEBUG_MODE "Build Debug" ON)
option(ARCH_64 "64-bit" ON)

if(DEBUG_MODE)
  set(CMAKE_BUILD_TYPE DEBUG)
else(DEBUG_MODE)
  set(CMAKE_BUILD_TYPE RELEASE)
endif(DEBUG_MODE)

set(CMAKE_C_FLAGS_DEBUG "-DSEN_DEBUG=1")
set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})


set(SRC_ROOT ${CMAKE_SOURCE_DIR}/src)
add_subdirectory ( ${SRC_ROOT} )


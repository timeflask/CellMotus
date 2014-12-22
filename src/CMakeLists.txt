cmake_minimum_required(VERSION 2.6)

set(CORE_ROOT ${SRC_ROOT}/core)
set(EXT_ROOT ${SRC_ROOT}/ext)

set(CMAKE_C_FLAGS_DEBUG "-DSEN_DEBUG=1")
set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})

if(WIN32) 
   set(PLATFORM_FOLDER win32)
elseif(APPLE)
   set(PLATFORM_FOLDER mac)
else() 
   set(PLATFORM_FOLDER linux)
endif()

include_directories(
  ${CORE_ROOT}
  ${EXT_ROOT}/ft2/include/${PLATFORM_FOLDER}
  ${EXT_ROOT}/png/include/${PLATFORM_FOLDER}
  ${EXT_ROOT}/luajit/include
)

link_directories(
  ${EXT_ROOT}/ft2/libs/${PLATFORM_FOLDER}
  ${EXT_ROOT}/png/libs/${PLATFORM_FOLDER}
  ${EXT_ROOT}/luajit/libs/${PLATFORM_FOLDER}
)

add_subdirectory( ${CORE_ROOT} )
add_subdirectory( ${SRC_ROOT}/${PLATFORM_FOLDER} )

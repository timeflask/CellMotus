cmake_minimum_required(VERSION 2.6)

set(CMAKE_C_FLAGS_DEBUG "-g -Wall -DSEN_DEBUG=1")
set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions -std=c99 -D _POSIX_C_SOURCE=200809L")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -std=c++11 -Wno-deprecated-declarations -Wno-reorder")
if(CLANG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

if ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  set(CMAKE_C_LINK_FLAGS "-pagezero_size 10000, -image_base 100000000") 
  set(CMAKE_CXX_LINK_FLAGS "-pagezero_size 10000, -image_base 100000000") 
endif()  


include_directories(
  ${EXT_ROOT}/glfw3/include/${PLATFORM_FOLDER}
)
link_directories(
  ${EXT_ROOT}/glfw3/libs/${PLATFORM_FOLDER}
)

set(PLATFORM_SOURCES 
  ${CORE_ROOT}/x_glfw_desktop_app.h
  ${CORE_ROOT}/x_glfw_desktop_app.c
  ${CORE_ROOT}/x_apple_audio.m
  ${CORE_ROOT}/x_posix_timer.c
  ${CORE_ROOT}/x_mac.c
  ${SRC_ROOT}/mac/main.c
)

find_library(MGLFW_LIB glfw3
PATHS
  ${EXT_ROOT}/glfw3/libs/${PLATFORM_FOLDER}
)

FIND_LIBRARY(COCOA_LIBRARY Cocoa)
FIND_LIBRARY(IOKIT_LIBRARY IOKit)
FIND_LIBRARY(CV_LIBRARY CoreVideo)

find_library(PNG_LIB png
PATHS
  ${EXT_ROOT}/png/libs/${PLATFORM_FOLDER}
)

find_library(FT2_LIB freetype freetype250
PATHS
  ${EXT_ROOT}/ft2/libs/${PLATFORM_FOLDER}
  ${EXT_ROOT}/ft2/libs/${PLATFORM_FOLDER}/${ARCH_DIR}
)

find_library(LUA_LIB luajit
PATHS
  ${EXT_ROOT}/luajit/libs/${PLATFORM_FOLDER}
)

find_package(ZLIB REQUIRED)
find_package(OpenGL REQUIRED)
find_library(AL_LIB OpenAL)
find_library(ATB_LIB AudioToolbox)
  
set(PLATFORM_LIBS 
   ${MGLFW_LIB} 
   ${COCOA_LIBRARY}
   ${IOKIT_LIBRARY}
   ${CV_LIBRARY}
   ${PNG_LIB} 
   ${FT2_LIB} 
   ${ZLIB_LIBRARY} 
   ${LUA_LIB}
   ${OPENGL_LIBRARIES}
   ${AL_LIB}
   ${ATB_LIB}
 )

add_executable(${APP_NAME}
 ${CORE_SOURCES}
 ${PLATFORM_SOURCES}
)

target_link_libraries(${APP_NAME} ${PLATFORM_LIBS})

set_target_properties(${APP_NAME} PROPERTIES
                         RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_custom_command(TARGET ${APP_NAME} POST_BUILD        
     COMMAND ${CMAKE_COMMAND} -E copy_if_different  
         "$<TARGET_FILE_DIR:${APP_NAME}>/$<TARGET_FILE_NAME:${APP_NAME}>"
         "${CMAKE_SOURCE_DIR}/bin/${PLATFORM_FOLDER}/$<TARGET_FILE_NAME:${APP_NAME}>") 


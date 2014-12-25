cmake_minimum_required(VERSION 2.6)

set(CMAKE_C_FLAGS_DEBUG "-g -Wall -DSEN_DEBUG=1")
set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions -std=c99 -D _POSIX_C_SOURCE=200809L")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -std=c++11 -Wno-deprecated-declarations -Wno-reorder")
if(CLANG)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

include_directories(
  ${EXT_ROOT}/glfw3/include/${PLATFORM_FOLDER}
  ${EXT_ROOT}/fmod/include/${ARCH_DIR}
)
link_directories(
  ${EXT_ROOT}/ft2/libs/${PLATFORM_FOLDER}/${ARCH_DIR}
  ${EXT_ROOT}/fmod/libs/${ARCH_DIR}
)

set(PLATFORM_SOURCES 
  ${CORE_ROOT}/x_glfw_desktop_app.h
  ${CORE_ROOT}/x_glfw_desktop_app.c
  ${CORE_ROOT}/x_linux_audio.cpp
  ${CORE_ROOT}/x_posix_timer.c
  ${CORE_ROOT}/x_posix.c
  ${CORE_ROOT}/x_file_settings.c
  ${SRC_ROOT}/linux/main.c
)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_library(LUAJIT_LIB luajit-5.1)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)
find_library(GLFW_LIB glfw3)
set(FREETYPE_LIBRARY   ${EXT_ROOT}/ft2/libs/linux/${ARCH_DIR}/libfreetype.a)

if ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
set(FMOD_LIB "fmodex64")
else()
set(FMOD_LIB "fmodex")
endif()

set(CORE_LIBS ${PNG_LIBRARY} ${FREETYPE_LIBRARY} ${ZLIB_LIBRARY} ${LUAJIT_LIB} ${GLFW_LIB} )

set(PLATFORM_LIBS 
  ${PNG_LIBRARY} 
  ${FREETYPE_LIBRARY} 
  ${ZLIB_LIBRARY} 
  ${LUAJIT_LIB} 
  ${GLFW_LIB}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES} 
  ${FMOD_LIB} 

  X11 Xrandr Xinerama Xi Xxf86vm Xcursor pthread m
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


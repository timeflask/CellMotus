cmake_minimum_required(VERSION 2.6)
set(APP_NAME CellMotus)
project (${APP_NAME})


find_library(LUAJIT_LIB luajit-5.1)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)

################ Options #########################
option(DEBUG_MODE "Debug || Release" ON)
##################################################

if(DEBUG_MODE)
  set(CMAKE_BUILD_TYPE DEBUG)
else(DEBUG_MODE)
  set(CMAKE_BUILD_TYPE RELEASE)
endif(DEBUG_MODE)

set(CMAKE_C_FLAGS_DEBUG "-g -Wall -DSEN_DEBUG=1")
set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11")

if ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
set(ARCH_DIR "64-bit")
else()
set(ARCH_DIR "32-bit")
endif()

set(SRC_ROOT ${CMAKE_SOURCE_DIR}/src)
set(CORE_ROOT ${CMAKE_SOURCE_DIR}/src/core)
set(EXT_ROOT ${CMAKE_SOURCE_DIR}/src/ext)

include_directories(
  ${CORE_ROOT}
  ${EXT_ROOT}/ft2/include/linux
  ${EXT_ROOT}/luajit/include
  ${EXT_ROOT}/fmod/include/${ARCH_DIR}
)

link_directories(
  /usr/local/lib
  /usr/lib
  ${EXT_ROOT}/ft2/libs/linux/${ARCH_DIR}
  ${EXT_ROOT}/fmod/libs/${ARCH_DIR}

)
set(FREETYPE_LIBRARY   ${EXT_ROOT}/ft2/libs/linux/${ARCH_DIR}/libfreetype.a)

if ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
set(FMOD_LIB "fmodex64")
else()
set(FMOD_LIB "fmodex")
endif()


set(GAME_SOURCES
  ${SRC_ROOT}/linux/main.c

  ${CORE_ROOT}/config.h
  ${CORE_ROOT}/macro.h
  ${CORE_ROOT}/opengl.h
  ${CORE_ROOT}/logger.h
  ${CORE_ROOT}/x_stdio_logger.c
  ${CORE_ROOT}/object.h
  ${CORE_ROOT}/object.c
  ${CORE_ROOT}/utils.h
  ${CORE_ROOT}/utils.c
  ${CORE_ROOT}/vector.h
  ${CORE_ROOT}/vector.c
  ${CORE_ROOT}/engine.h
  ${CORE_ROOT}/engine.c
  ${CORE_ROOT}/hrtimer.h
  ${CORE_ROOT}/x_posix_timer.c
  ${CORE_ROOT}/lmath.h
  ${CORE_ROOT}/lmath.c
  ${CORE_ROOT}/asset.h
  ${CORE_ROOT}/x_file_asset.c
  ${CORE_ROOT}/vertex-attribute.h
  ${CORE_ROOT}/vertex-attribute.c
  ${CORE_ROOT}/vertex-buffer.h
  ${CORE_ROOT}/vertex-buffer.c
  ${CORE_ROOT}/shader.h
  ${CORE_ROOT}/shader.c
  ${CORE_ROOT}/texture-atlas.h
  ${CORE_ROOT}/texture-atlas.c
  ${CORE_ROOT}/image.h
  ${CORE_ROOT}/image.c
  ${CORE_ROOT}/texture.h
  ${CORE_ROOT}/texture.c
  ${CORE_ROOT}/texture-manager.h
  ${CORE_ROOT}/texture-manager.c
  ${CORE_ROOT}/edtaa3func.h
  ${CORE_ROOT}/edtaa3func.c
  ${CORE_ROOT}/node.h
  ${CORE_ROOT}/node.c
  ${CORE_ROOT}/sprite.h
  ${CORE_ROOT}/sprite.c
  ${CORE_ROOT}/view.h
  ${CORE_ROOT}/view.c
  ${CORE_ROOT}/camera.h
  ${CORE_ROOT}/camera.c
  ${CORE_ROOT}/signals.h
  ${CORE_ROOT}/signals.c
  ${CORE_ROOT}/font.h
  ${CORE_ROOT}/font.c
  ${CORE_ROOT}/label.h
  ${CORE_ROOT}/label.c
  ${CORE_ROOT}/scheduler.h
  ${CORE_ROOT}/scheduler.c
  ${CORE_ROOT}/luas.h
  ${CORE_ROOT}/luas.c
  ${CORE_ROOT}/scene.h
  ${CORE_ROOT}/scene.c
  ${CORE_ROOT}/layer.h
  ${CORE_ROOT}/layer.c
  ${CORE_ROOT}/render.h
  ${CORE_ROOT}/render.c
  ${CORE_ROOT}/input.h
  ${CORE_ROOT}/input.c
  ${CORE_ROOT}/shapes.h
  ${CORE_ROOT}/shapes.c
  ${CORE_ROOT}/quad.h
  ${CORE_ROOT}/quad.c
  ${CORE_ROOT}/platform.h
  ${CORE_ROOT}/x_posix.c
  ${CORE_ROOT}/audio.h
  ${CORE_ROOT}/x_linux_audio.cpp
  ${CORE_ROOT}/settings.h
  ${CORE_ROOT}/x_file_settings.c
  ${CORE_ROOT}/shaders/vs_pos4f_tex2f.h
  ${CORE_ROOT}/shaders/fs_tex2f.h
  ${CORE_ROOT}/shaders/vs_pos4f_tex2f_col4f.h
  ${CORE_ROOT}/shaders/fs_tex2f_col4f.h
  ${CORE_ROOT}/shaders/fs_text_tex2f_col4f.h
)


add_executable(${APP_NAME}
${GAME_SOURCES}
)

set(APP_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
set_target_properties(${APP_NAME} PROPERTIES
     RUNTIME_OUTPUT_DIRECTORY  "${APP_BIN_DIR}")

target_link_libraries(${APP_NAME} ${PNG_LIBRARY} ${FREETYPE_LIBRARY} ${ZLIB_LIBRARY} ${LUAJIT_LIB} ${FMOD_LIB}  m glut GL GLU GLEW)


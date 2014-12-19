cmake_minimum_required(VERSION 2.6)

#find_library(GL_LIB GL)

set(APP_NAME CellMotus)
set(SRC_ROOT ${CMAKE_SOURCE_DIR}/src)
set(CORE_ROOT ${CMAKE_SOURCE_DIR}/src/core)
set(EXT_ROOT ${CMAKE_SOURCE_DIR}/src/ext)
 
project (${APP_NAME})

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  if(WINRT)
    set(SYSTEM_STRING "Windows RT")
  elseif(WP8)
    set(SYSTEM_STRING "Windows Phone 8")
  else()
    set(WINDOWS TRUE)
    set(SYSTEM_STRING "Windows Desktop")
  endif()
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  if(ANDROID)
    set(SYSTEM_STRING "Android")
  else()
    set(LINUX TRUE)
    set(SYSTEM_STRING "Linux")
  endif()
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  if(IOS)
    set(SYSTEM_STRING "IOS")
  else()
    set(MACOSX TRUE)
    set(APPLE TRUE)
    set(SYSTEM_STRING "Mac OSX")
  endif()
endif()

message (${SYSTEM_STRING})

set(CMAKE_C_FLAGS_DEBUG "-DSEN_DEBUG=1")
set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})

if(MSVC)
  ADD_DEFINITIONS (-D_WINDOWS -DWIN32)

  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS
                  -wd4251 -wd4244 -wd4334 -wd4005 -wd4820 -wd4710
                  -wd4514 -wd4056 -wd4996 -wd4099)
else()
  set(CMAKE_C_FLAGS_DEBUG "-g -Wall -DSEN_DEBUG=1")
  set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions -std=c99")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -std=c++11 -Wno-deprecated-declarations -Wno-reorder")
  if(CLANG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  endif()
endif(MSVC)


############### Options #########################
#option(DEBUG_MODE "Debug || Release" ON)
#################################################

#if(DEBUG_MODE)
 # set(CMAKE_BUILD_TYPE DEBUG)
#else(DEBUG_MODE)
 # set(CMAKE_BUILD_TYPE RELEASE)
#endif(DEBUG_MODE)

set(SRC_ROOT ${CMAKE_SOURCE_DIR}/src)
set(CORE_ROOT ${CMAKE_SOURCE_DIR}/src/core)
set(EXT_ROOT ${CMAKE_SOURCE_DIR}/src/ext)

if(WIN32) 
   set(PLATFORM_FOLDER win32)
elseif(APPLE)
   set(PLATFORM_FOLDER mac)
else() 
   set(PLATFORM_FOLDER linux)
endif()

if ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  set(ARCH_DIR "64-bit")
else()
  set(ARCH_DIR "32-bit")
endif()

# -------------------------------------------------------------------- includes
include_directories(
  ${CORE_ROOT}
  ${EXT_ROOT}/ft2/include/${PLATFORM_FOLDER}
  ${EXT_ROOT}/png/include/${PLATFORM_FOLDER}
  ${EXT_ROOT}/luajit/include
)


# -------------------------------------------------------------------- link

link_directories(
  ${EXT_ROOT}/ft2/libs/${PLATFORM_FOLDER}
  ${EXT_ROOT}/png/libs/${PLATFORM_FOLDER}
  ${EXT_ROOT}/luajit/libs/${PLATFORM_FOLDER}
)



set(CORE_SOURCES
  ${CORE_ROOT}/config.h
  ${CORE_ROOT}/macro.h
  ${CORE_ROOT}/opengl.h
  ${CORE_ROOT}/logger.h
  ${CORE_ROOT}/object.h
  ${CORE_ROOT}/object.c
  ${CORE_ROOT}/utils.h
  ${CORE_ROOT}/utils.c
  ${CORE_ROOT}/vector.h
  ${CORE_ROOT}/vector.c
  ${CORE_ROOT}/engine.h
  ${CORE_ROOT}/engine.c
  ${CORE_ROOT}/hrtimer.h
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
  ${CORE_ROOT}/audio.h
  ${CORE_ROOT}/settings.h
  ${CORE_ROOT}/x_file_settings.c
  ${CORE_ROOT}/shaders/vs_pos4f_tex2f.h
  ${CORE_ROOT}/shaders/fs_tex2f.h
  ${CORE_ROOT}/shaders/vs_pos4f_tex2f_col4f.h
  ${CORE_ROOT}/shaders/fs_tex2f_col4f.h
  ${CORE_ROOT}/shaders/fs_text_tex2f_col4f.h

  ${CORE_ROOT}/x_stdio_logger.c
)

if(WIN32) 
include_directories(
  ${EXT_ROOT}/zlib/include
  ${EXT_ROOT}/glew/include
  ${EXT_ROOT}/glfw3/include/${PLATFORM_FOLDER}
)
link_directories(
  ${EXT_ROOT}/zlib/libs
  ${EXT_ROOT}/glew/libs
  ${EXT_ROOT}/glfw3/libs/${PLATFORM_FOLDER}
)

set(PLATFORM_SOURCES 
  ${CORE_ROOT}/x_glfw_desktop_app.h
  ${CORE_ROOT}/x_glfw_desktop_app.c
  ${CORE_ROOT}/x_win32_audio.c
  ${CORE_ROOT}/x_win32_timer.c
  ${CORE_ROOT}/x_win32.c
)
  
set(GAME_SOURCES 
  ${SRC_ROOT}/win32/main.c
)
  
find_library(WIN32Z_LIB zlib
 PATHS
   ${EXT_ROOT}/zlib/libs
)

find_library(WIN32GLFW_LIB glfw3
 PATHS
   ${EXT_ROOT}/glfw3/libs/${PLATFORM_FOLDER}
)
  
 set(PLATFORM_LIBS 
   ${WIN32Z_LIB}
   ${WIN32GLFW_LIB}
)

elseif(APPLE)

else() 
include_directories(
  ${EXT_ROOT}/glfw3/include/${PLATFORM_FOLDER}
  ${EXT_ROOT}/fmod/include/${ARCH_DIR}
)
link_directories(
#  /usr/local/lib
  ${EXT_ROOT}/ft2/libs/${PLATFORM_FOLDER}/${ARCH_DIR}
  ${EXT_ROOT}/fmod/libs/${ARCH_DIR}
)

set(PLATFORM_SOURCES 
  ${CORE_ROOT}/x_glfw_desktop_app.h
  ${CORE_ROOT}/x_glfw_desktop_app.c
  ${CORE_ROOT}/x_linux_audio.cpp
  ${CORE_ROOT}/x_posix_timer.c
  ${CORE_ROOT}/x_posix.c
)
  
set(GAME_SOURCES 
  ${SRC_ROOT}/linux/main.c)
endif()


if(WINDOWS)     
  SET(CMAKE_FIND_LIBRARY_PREFIXES "")
  SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
elseif(LINUX)  
#  SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
else()  
endif()

if(WINDOWS)
  find_library(PNG_LIB libpng
  PATHS
   ${EXT_ROOT}/png/libs/${PLATFORM_FOLDER}
  )

  find_library(FT2_LIB freetype freetype250
  PATHS
   ${EXT_ROOT}/ft2/libs/${PLATFORM_FOLDER}
   ${EXT_ROOT}/ft2/libs/${PLATFORM_FOLDER}/${ARCH_DIR}
  )

  find_library(LUA_LIB lualib lua51
  PATHS
   ${EXT_ROOT}/luajit/libs/${PLATFORM_FOLDER}
  )
   
  set(CORE_LIBS ${PNG_LIB} ${FT2_LIB} ${LUA_LIB})
else()
  find_library(LUAJIT_LIB luajit-5.1)
  find_package(PNG REQUIRED)
  find_package(ZLIB REQUIRED)
  find_library(GLFW_LIB glfw3)
  set(FREETYPE_LIBRARY   ${EXT_ROOT}/ft2/libs/linux/${ARCH_DIR}/libfreetype.a)
  set(CORE_LIBS ${PNG_LIBRARY} ${FREETYPE_LIBRARY} ${ZLIB_LIBRARY} ${LUAJIT_LIB} ${GLFW_LIB} )
endif()

if(LINUX OR MACOSX OR WINDOWS)
  find_package(OpenGL REQUIRED)

  set(PLATFORM_LIBS ${PLATFORM_LIBS} ${OPENGL_LIBRARIES})
  
  if(LINUX)
    find_package(GLEW REQUIRED)
    if ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
      set(FMOD_LIB "fmodex64")
    else()
      set(FMOD_LIB "fmodex")
    endif()
    set(PLATFORM_LIBS ${PLATFORM_LIBS} ${GLEW_LIBRARIES} ${FMOD_LIB} X11 Xrandr Xinerama Xi Xxf86vm Xcursor pthread m)
  elseif(WINDOWS)  
    set(GLEW_LIB ${EXT_ROOT}/glew/libs/glew32.lib )
    message(${GLEW_LIB})
    set(PLATFORM_LIBS ${PLATFORM_LIBS} ${GLEW_LIB})
  endif()
endif()

message(${CORE_LIBS})
message(${PLATFORM_LIBS})

add_executable(${APP_NAME}
 ${CORE_SOURCES}
 ${GAME_SOURCES}
 ${PLATFORM_SOURCES}
)
set(APP_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
set_target_properties(${APP_NAME} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY  "${APP_BIN_DIR}"
      )
      
if(MSVC)
  if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(${APP_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
  endif() 
endif()
     
target_link_libraries(${APP_NAME} ${CORE_LIBS} ${PLATFORM_LIBS})

# set(APP_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
# set_target_properties(${APP_NAME} PROPERTIES
     # RUNTIME_OUTPUT_DIRECTORY  "${APP_BIN_DIR}")

# target_link_libraries(${APP_NAME} ${PNG_LIBRARY} ${FREETYPE_LIBRARY} ${ZLIB_LIBRARY} ${LUAJIT_LIB} ${FMOD_LIB}  m glut GL GLU GLEW)

# set(GAME_SOURCES
  # ${SRC_ROOT}/win32/main.c
# )
 
# add_executable(${APP_NAME} ${GAME_SOURCES})

# find_library(LUAJIT_LIB luajit-5.1)
# find_package(PNG REQUIRED)
# find_package(ZLIB REQUIRED)

# set(CMAKE_C_FLAGS_DEBUG "-g -Wall -DSEN_DEBUG=1")
# set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
# set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
# set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11")

# link_directories(
  # /usr/local/lib
  # /usr/lib
  # ${EXT_ROOT}/ft2/libs/linux/${ARCH_DIR}
  # ${EXT_ROOT}/fmod/libs/${ARCH_DIR}

# )
# set(FREETYPE_LIBRARY   ${EXT_ROOT}/ft2/libs/linux/${ARCH_DIR}/libfreetype.a)

# if ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
# set(FMOD_LIB "fmodex64")
# else()
# set(FMOD_LIB "fmodex")
# endif()


# set(GAME_SOURCES
  # ${SRC_ROOT}/linux/main.c

  # ${CORE_ROOT}/config.h
  # ${CORE_ROOT}/macro.h
  # ${CORE_ROOT}/opengl.h
  # ${CORE_ROOT}/logger.h
  # ${CORE_ROOT}/x_stdio_logger.c
  # ${CORE_ROOT}/object.h
  # ${CORE_ROOT}/object.c
  # ${CORE_ROOT}/utils.h
  # ${CORE_ROOT}/utils.c
  # ${CORE_ROOT}/vector.h
  # ${CORE_ROOT}/vector.c
  # ${CORE_ROOT}/engine.h
  # ${CORE_ROOT}/engine.c
  # ${CORE_ROOT}/hrtimer.h
  # ${CORE_ROOT}/x_posix_timer.c
  # ${CORE_ROOT}/lmath.h
  # ${CORE_ROOT}/lmath.c
  # ${CORE_ROOT}/asset.h
  # ${CORE_ROOT}/x_file_asset.c
  # ${CORE_ROOT}/vertex-attribute.h
  # ${CORE_ROOT}/vertex-attribute.c
  # ${CORE_ROOT}/vertex-buffer.h
  # ${CORE_ROOT}/vertex-buffer.c
  # ${CORE_ROOT}/shader.h
  # ${CORE_ROOT}/shader.c
  # ${CORE_ROOT}/texture-atlas.h
  # ${CORE_ROOT}/texture-atlas.c
  # ${CORE_ROOT}/image.h
  # ${CORE_ROOT}/image.c
  # ${CORE_ROOT}/texture.h
  # ${CORE_ROOT}/texture.c
  # ${CORE_ROOT}/texture-manager.h
  # ${CORE_ROOT}/texture-manager.c
  # ${CORE_ROOT}/edtaa3func.h
  # ${CORE_ROOT}/edtaa3func.c
  # ${CORE_ROOT}/node.h
  # ${CORE_ROOT}/node.c
  # ${CORE_ROOT}/sprite.h
  # ${CORE_ROOT}/sprite.c
  # ${CORE_ROOT}/view.h
  # ${CORE_ROOT}/view.c
  # ${CORE_ROOT}/camera.h
  # ${CORE_ROOT}/camera.c
  # ${CORE_ROOT}/signals.h
  # ${CORE_ROOT}/signals.c
  # ${CORE_ROOT}/font.h
  # ${CORE_ROOT}/font.c
  # ${CORE_ROOT}/label.h
  # ${CORE_ROOT}/label.c
  # ${CORE_ROOT}/scheduler.h
  # ${CORE_ROOT}/scheduler.c
  # ${CORE_ROOT}/luas.h
  # ${CORE_ROOT}/luas.c
  # ${CORE_ROOT}/scene.h
  # ${CORE_ROOT}/scene.c
  # ${CORE_ROOT}/layer.h
  # ${CORE_ROOT}/layer.c
  # ${CORE_ROOT}/render.h
  # ${CORE_ROOT}/render.c
  # ${CORE_ROOT}/input.h
  # ${CORE_ROOT}/input.c
  # ${CORE_ROOT}/shapes.h
  # ${CORE_ROOT}/shapes.c
  # ${CORE_ROOT}/quad.h
  # ${CORE_ROOT}/quad.c
  # ${CORE_ROOT}/platform.h
  # ${CORE_ROOT}/x_posix.c
  # ${CORE_ROOT}/audio.h
  # ${CORE_ROOT}/x_linux_audio.cpp
  # ${CORE_ROOT}/settings.h
  # ${CORE_ROOT}/x_file_settings.c
  # ${CORE_ROOT}/shaders/vs_pos4f_tex2f.h
  # ${CORE_ROOT}/shaders/fs_tex2f.h
  # ${CORE_ROOT}/shaders/vs_pos4f_tex2f_col4f.h
  # ${CORE_ROOT}/shaders/fs_tex2f_col4f.h
  # ${CORE_ROOT}/shaders/fs_text_tex2f_col4f.h
# )


# add_executable(${APP_NAME}
# ${GAME_SOURCES}
# )

# set(APP_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
# set_target_properties(${APP_NAME} PROPERTIES
     # RUNTIME_OUTPUT_DIRECTORY  "${APP_BIN_DIR}")

# target_link_libraries(${APP_NAME} ${PNG_LIBRARY} ${FREETYPE_LIBRARY} ${ZLIB_LIBRARY} ${LUAJIT_LIB} ${FMOD_LIB}  m glut GL GLU GLEW)



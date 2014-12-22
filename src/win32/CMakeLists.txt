cmake_minimum_required(VERSION 2.6)

ADD_DEFINITIONS (-D_WINDOWS -DWIN32)
ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS
                -wd4251 -wd4244 -wd4334 -wd4005 -wd4820 -wd4710
                -wd4514 -wd4056 -wd4996 -wd4099)

include_directories(
  ${EXT_ROOT}/zlib/include
  ${EXT_ROOT}/glew/include
  ${EXT_ROOT}/glfw3/include/win32
)
link_directories(
  ${EXT_ROOT}/zlib/libs
  ${EXT_ROOT}/glew/libs
  ${EXT_ROOT}/glfw3/libs/win32
)

set(PLATFORM_SOURCES 
  ${CORE_ROOT}/x_glfw_desktop_app.h
  ${CORE_ROOT}/x_glfw_desktop_app.c
  ${CORE_ROOT}/x_win32_audio.c
  ${CORE_ROOT}/x_win32_timer.c
  ${CORE_ROOT}/x_win32.c
  ${SRC_ROOT}/win32/main.c
)
  
SET(CMAKE_FIND_LIBRARY_PREFIXES "")
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")

find_library(PNG_LIB libpng
PATHS
  ${EXT_ROOT}/png/libs/win32
)

find_library(FT2_LIB freetype freetype250
PATHS
  ${EXT_ROOT}/ft2/libs/win32
)

find_library(LUA_LIB lualib lua51
PATHS
  ${EXT_ROOT}/luajit/libs/win32
)
   
find_library(GLEW_LIB glew32
PATHS
  ${EXT_ROOT}/glew/libs
)
   
find_package(OpenGL REQUIRED)

find_library(Z_LIB zlib
 PATHS
   ${EXT_ROOT}/zlib/libs
)

find_library(GLFW_LIB glfw3dll
 PATHS
   ${EXT_ROOT}/glfw3/libs/win32
)

set(PLATFORM_LIBS ${Z_LIB} ${PNG_LIB} ${FT2_LIB} ${LUA_LIB} ${OPENGL_LIBRARIES} ${GLEW_LIB} ${GLFW_LIB})

add_executable(${APP_NAME}
 ${CORE_SOURCES}
 ${PLATFORM_SOURCES}
)

target_link_libraries(${APP_NAME} ${PLATFORM_LIBS})

if(DEBUG_MODE)
  set_target_properties(${APP_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE")
else(DEBUG_MODE)
  set_target_properties(${APP_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
endif(DEBUG_MODE)

set_target_properties(${APP_NAME} PROPERTIES
                         RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")


add_custom_command(TARGET ${APP_NAME} POST_BUILD        
     COMMAND ${CMAKE_COMMAND} -E copy_if_different  
        "${CMAKE_SOURCE_DIR}/bin/${PLATFORM_FOLDER}/lua51.dll"
         $<TARGET_FILE_DIR:${APP_NAME}>) 
add_custom_command(TARGET ${APP_NAME} POST_BUILD        
     COMMAND ${CMAKE_COMMAND} -E copy_if_different  
        "${CMAKE_SOURCE_DIR}/bin/${PLATFORM_FOLDER}/glew32.dll"
         $<TARGET_FILE_DIR:${APP_NAME}>) 
add_custom_command(TARGET ${APP_NAME} POST_BUILD        
     COMMAND ${CMAKE_COMMAND} -E copy_if_different  
        "${CMAKE_SOURCE_DIR}/bin/${PLATFORM_FOLDER}/glfw3.dll"
         $<TARGET_FILE_DIR:${APP_NAME}>) 

add_custom_command(TARGET ${APP_NAME} POST_BUILD        
     COMMAND ${CMAKE_COMMAND} -E copy_if_different  
         "$<TARGET_FILE_DIR:${APP_NAME}>/$<TARGET_FILE_NAME:${APP_NAME}>"
         "${CMAKE_SOURCE_DIR}/bin/${PLATFORM_FOLDER}/$<TARGET_FILE_NAME:${APP_NAME}>") 

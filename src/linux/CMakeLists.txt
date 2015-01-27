cmake_minimum_required(VERSION 2.6)
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/install/linux/")

option(LUA_STATIC "LUA_STATIC" ON)

set(CMAKE_C_FLAGS_DEBUG "-g -Wall -DSEN_DEBUG=1")
set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set(CMAKE_CPP_FLAGS "${CMAKE_CPP_FLAGS} -D_FORTIFY_SOURCE=2")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions -std=c99 -D _POSIX_C_SOURCE=200809L -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -D_FORTIFY_SOURCE=2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -std=c++11 -Wno-deprecated-declarations -Wno-reorder -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -D_FORTIFY_SOURCE=2")

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions -std=c99 -D _POSIX_C_SOURCE=200809L -g -O2")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -std=c++11 -Wno-deprecated-declarations -Wno-reorder -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -D_FORTIFY_SOURCE=2")


if(CLANG)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro")


include_directories(
  ${EXT_ROOT}/glfw3/include/${PLATFORM_FOLDER}
  ${EXT_ROOT}/glew/include
)
link_directories(
 /usr/lib
  #${EXT_ROOT}/ft2/libs/${PLATFORM_FOLDER}/${ARCH_DIR}
#  ${EXT_ROOT}/fmod/libs/${ARCH_DIR}
)

set(PLATFORM_SOURCES 
  ${CORE_ROOT}/x_glfw_desktop_app.h
  ${CORE_ROOT}/x_glfw_desktop_app.c
  ${CORE_ROOT}/x_openal_audio.c
  ${CORE_ROOT}/x_posix_timer.c
  ${CORE_ROOT}/x_posix.c
  ${CORE_ROOT}/x_file_settings.c
  ${CORE_ROOT}/x_file_asset.c
  ${SRC_ROOT}/linux/main.c
  ${EXT_ROOT}/glew/glew.c
)

find_package(OpenGL REQUIRED)
#find_package(GLEW REQUIRED)
find_package(OpenAL REQUIRED)
find_library(ALUT_LIB alut)

include_directories(
  ${OPENAL_INCLUDE_DIR}/../
  ${OPENAL_INCLUDE_DIR}
 )

find_package(PNG REQUIRED)
#find_package(Luajit REQUIRED)
#find_package(ZLIB REQUIRED)

#find_library(GLFW_LIB glfw3)

if ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  set (GLFW_LIB ${EXT_ROOT}/glfw3/libs/linux/64-bit/libglfw3.a)
else()
  set (GLFW_LIB ${EXT_ROOT}/glfw3/libs/linux/32-bit/libglfw3.a)
endif()

if(LUA_STATIC)
	if ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	  set (LUAJIT_LIB ${EXT_ROOT}/luajit/libs/linux/64-bit/libluajit.a)
	else()
	  set (LUAJIT_LIB ${EXT_ROOT}/luajit/libs/linux/32-bit/libluajit.a)
	endif()
include_directories(
  ${EXT_ROOT}/luajit/include
)

else(LUA_STATIC)
  find_library(LUAJIT_LIB luajit-5.1)
include_directories(
  ${EXT_ROOT}/luajit/include
)

endif(LUA_STATIC)



#set(FREETYPE_LIBRARY   ${EXT_ROOT}/ft2/libs/linux/${ARCH_DIR}/libfreetype.a)
find_package(Freetype REQUIRED)
include_directories(
  /usr/include/freetype2  
  /usr/local/include/freetype2  
 )

#set(CORE_LIBS ${PNG_LIBRARY} ${FREETYPE_LIBRARY} ${ZLIB_LIBRARY} ${LUAJIT_LIB} ${GLFW_LIB} )

set(PLATFORM_LIBS 
  ${LUAJIT_LIB} 
  ${PNG_LIBRARY} 
  ${FREETYPE_LIBRARY} 
  ${GLFW_LIB}
  ${OPENAL_LIBRARY} 
  ${ALUT_LIB}

  GL
  X11 
  Xrandr 
  Xi 
  Xxf86vm 
  pthread 
  m
  dl
)

file(GLOB assets
    "${CMAKE_BINARY_DIR}/assets/*.*"
)

add_executable(${APP_NAME}
 ${CORE_SOURCES}
 ${PLATFORM_SOURCES}
)
message(${PLATFORM_LIBS})
target_link_libraries(${APP_NAME} ${PLATFORM_LIBS})

set_target_properties(${APP_NAME} PROPERTIES
                         RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_custom_command(TARGET ${APP_NAME} POST_BUILD        
     COMMAND ${CMAKE_COMMAND} -E copy_if_different  
         "$<TARGET_FILE_DIR:${APP_NAME}>/$<TARGET_FILE_NAME:${APP_NAME}>"
         "${CMAKE_SOURCE_DIR}/bin/${PLATFORM_FOLDER}/$<TARGET_FILE_NAME:${APP_NAME}>") 


install(TARGETS ${APP_NAME} DESTINATION /usr/games COMPONENT binaries)
install(FILES ${CMAKE_BINARY_DIR}/install/linux/${APP_NAME}.desktop DESTINATION /usr/share/applications/)
install(DIRECTORY ${CMAKE_BINARY_DIR}/assets DESTINATION /usr/share/${APP_NAME})

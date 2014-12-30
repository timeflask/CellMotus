cmake_minimum_required(VERSION 2.8)
set(CMAKE_C_FLAGS_DEBUG "-g -Wall -DSEN_DEBUG=1")

set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions -std=c99 -D _POSIX_C_SOURCE=200809L")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -std=c++11 -Wno-deprecated-declarations -Wno-reorder")

if(CLANG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()


if ( ARCH_64 )
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
  ${CORE_ROOT}/x_apple_settings.m
  ${SRC_ROOT}/mac/main.c
  ${SRC_ROOT}/mac/main.icns
  ${CORE_ROOT}/x_ios_asset.m
  )
  set_source_files_properties(main.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  
  set(MGLFW_LIB ${EXT_ROOT}/glfw3/libs/${PLATFORM_FOLDER}/${ARCH_DIR}/libglfw3.a)
  message(${MGLFW_LIB})
  #find_library(MGLFW_LIB glfw3
  #PATHS
  # ${EXT_ROOT}/glfw3/libs/${PLATFORM_FOLDER}/${ARCH_DIR}
  #)
  
  FIND_LIBRARY(COCOA_LIBRARY Cocoa)
  
  FIND_LIBRARY(IOKIT_LIBRARY IOKit)
  
  FIND_LIBRARY(CV_LIBRARY CoreVideo)
  #find_library(PNG_LIB png
  #PATHS
  # ${EXT_ROOT}/png/libs/${PLATFORM_FOLDER}/${ARCH_DIR}
  #)
  
  set(PNG_LIB ${EXT_ROOT}/png/libs/${PLATFORM_FOLDER}/${ARCH_DIR}/libpng.a)
  message(${PNG_LIB})
  
  find_library(FT2_LIB freetype freetype250
  PATHS
  ${EXT_ROOT}/ft2/libs/${PLATFORM_FOLDER}
  ${EXT_ROOT}/ft2/libs/${PLATFORM_FOLDER}/${ARCH_DIR}
  )
  
  #find_library(LUA_LIB luajit
  #PATHS
  # ${EXT_ROOT}/luajit/libs/${PLATFORM_FOLDER}/${ARCH_DIR}
  #)
  
  set(LUA_LIB ${EXT_ROOT}/luajit/libs/${PLATFORM_FOLDER}/${ARCH_DIR}/libluajit.a)
  message(${LUA_LIB})
  #find_package(PNG REQUIRED)
  #find_package(FREETYPE REQUIRED)
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

  
add_executable(
  ${APP_NAME} 
  MACOSX_BUNDLE
  ${CORE_SOURCES}
  ${PLATFORM_SOURCES}
)

target_link_libraries(${APP_NAME} ${PLATFORM_LIBS})

set_target_properties(${APP_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${SRC_ROOT}/mac/info.plist")
set_target_properties(${APP_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
  
set(PRODUCT_NAME \${TARGET_BUILD_DIR}/\${FULL_PRODUCT_NAME})
set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/assets)

# add_custom_command(
    # TARGET ${APP_NAME}
    # POST_BUILD
    # COMMAND mkdir -p ${PRODUCT_NAME}/Contents/Resources && cp -Rf ${ASSETS_DIR} ${PRODUCT_NAME}/Contents/Resources/
# )


ADD_CUSTOM_COMMAND(
   TARGET ${PROJECT_NAME}
   POST_BUILD
   COMMAND rsync
   ARGS -r --exclude '.DS_Store'
      "${ASSETS_DIR}"
      "${PRODUCT_NAME}/Contents/Resources/"
   COMMENT "Copying assets")
   
# ADD_CUSTOM_COMMAND(
   # TARGET ${PROJECT_NAME}
   # POST_BUILD
   # COMMAND rsync
   # ARGS -r --exclude '.DS_Store'
      # "${PRODUCT_NAME}"
      # "${CMAKE_SOURCE_DIR}/bin/${PLATFORM_FOLDER}/${APP_NAME}.app"
   # COMMENT "Release")
  
  
set(RELEASE_APP ${CMAKE_SOURCE_DIR}/bin/${PLATFORM_FOLDER}/${APP_NAME}.app)  
 add_custom_command(
     TARGET ${APP_NAME}
     POST_BUILD
     COMMAND rm -Rf ${RELEASE_APP} && cp -Raf ${PRODUCT_NAME} ${RELEASE_APP}
 )
 # add_custom_command(TARGET ${APP_NAME} POST_BUILD
   # COMMAND ${CMAKE_COMMAND} -E copy_if_different
   # "${PRODUCT_NAME}"
   # "${CMAKE_SOURCE_DIR}/bin/${PLATFORM_FOLDER}/${APP_NAME}.app")
  
       
          
#Project Definition 
cmake_minimum_required(VERSION 3.8)
project(modengine)

message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")

# Autoserialization code uses this extensively.  Should look into it, but supress since floods
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_FIND_LIBRARY_SUFFIXES .so .a)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  message("CMAKE - ModEngine Debug")
  
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")

  set(CMAKE_ENABLE_EXPORTS TRUE) # TODO -> modassert backtrace symbols
  add_definitions(-DMODDEBUG)

endif()
    
if (CMAKE_BUILD_TYPE STREQUAL "Release")
  message("CMAKE - ModEngine Release")
endif()

#Source files
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(BUILD_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib)

set (additional_source_list "")
if (ADDITIONAL_SRC)
  if(IS_ABSOLUTE ${ADDITIONAL_SRC})
    message(FATAL_ERROR "Source directory must be relative to root source dir, got : ${ADDITIONAL_SRC}")
  endif()

  set(ADDITIONAL_SRC_PATH "${SRC_DIR}/${ADDITIONAL_SRC}")
  set(ADDITIONAL_SRC_PATH_HEADER "${ADDITIONAL_SRC_PATH}/bindings.h")
  set(ADDITIONAL_SRC_PATH_GLOB "${ADDITIONAL_SRC_PATH}/*.cpp")
  message("include additional src directory: ${ADDITIONAL_SRC_PATH}, glob on ${ADDITIONAL_SRC_PATH_GLOB}")
  file(GLOB_RECURSE additional_source_list "${ADDITIONAL_SRC_PATH_GLOB}")
  message("addition source list: ${additional_source_list}, header is ${ADDITIONAL_SRC_PATH_HEADER}")

  if(NOT EXISTS "${ADDITIONAL_SRC}")
    message(FATAL_ERROR "Cannot include ADDITIONAL_SRC: ${ADDITIONAL_SRC} does not exist")
  endif()
  add_definitions("-DADDITIONAL_SRC_HEADER=${ADDITIONAL_SRC_PATH_HEADER}")
endif()


if (NOT BUILDTEST)
  file(GLOB_RECURSE source_list "${SRC_DIR}/*.cpp")
  add_executable(${PROJECT_NAME} ${source_list} ${additional_source_list})

else()
  message("Peforming Build Test")
  file(GLOB_RECURSE source_list "${CMAKE_CURRENT_SOURCE_DIR}/buildtest/*.cpp")

  set(NO_INCLUDE_GLFW TRUE)
  if (NOT NO_INCLUDE_GLFW)
    add_definitions(-DINCLUDE_GLFW)
  endif()

  set(NO_INCLUDE_GLAD TRUE)
  if (NOT NO_INCLUDE_GLAD)
    add_definitions(-DINCLUDE_GLAD)
  endif()

  set(NO_INCLUDE_OPENAL TRUE)
  if (NOT NO_INCLUDE_OPENAL)
    add_definitions(-DINCLUDE_OPENAL)
  endif()
  
  set(NO_INCLUDE_GLM TRUE)
  if (NOT NO_INCLUDE_GLM)
    add_definitions(-DINCLUDE_GLM)
  endif()

  set(NO_INCLUDE_STB TRUE)
  if (NOT NO_INCLUDE_STB)
    add_definitions(-DINCLUDE_STB)
  endif()

  set(NO_INCLUDE_CXXOPTS TRUE)
  if (NOT NO_INCLUDE_CXXOPTS)
    add_definitions(-DINCLUDE_CXXOPTS)
  endif()
  
  set(NO_INCLUDE_ASSIMP TRUE)
  if (NOT NO_INCLUDE_ASSIMP)
    add_definitions(-DINCLUDE_ASSIMP)
  endif()

  set(NO_INCLUDE_BULLET TRUE)
  if (NOT NO_INCLUDE_BULLET)
    add_definitions(-DINCLUDE_BULLET)
  endif()

  set(NO_INCLUDE_FREETYPE TRUE)
  if (NOT NO_INCLUDE_FREETYPE)
    add_definitions(-DINCLUDE_FREETYPE)
  endif()


  set(NO_INCLUDE_VIDEO FALSE)
  if (NOT NO_INCLUDE_VIDEO)
    add_definitions(-DINCLUDE_VIDEO)
  endif()

  add_executable(${PROJECT_NAME} ${source_list})
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} "stdc++fs")

# GLFW Library - window management 
if (NOT NO_INCLUDE_GLFW)
  message("MODBUILD - Including GLFW")
  set(GLFW_DIR "${LIB_DIR}/glfw-3.3")
  set(GLFW_BUILD_EXAMPLES   OFF   CACHE   INTERNAL   "build the glfw example programs")
  set(GLFW_BUILD_TESTS      OFF   CACHE   INTERNAL   "build the glfw test programs")
  add_subdirectory("${GLFW_DIR}")
  target_compile_definitions(${PROJECT_NAME} PRIVATE "GLFW_INCLUDE_NONE")   # makes glfw not include opengl headers.  Think more about this.
  target_link_libraries(${PROJECT_NAME} "glfw" "${GLFW_LIBRARIES}")
  target_include_directories(${PROJECT_NAME} PRIVATE "${GLFW_DIR}/include")
else()
  message("MODBUILD - Not Including GLFW")
endif()

# GLAD Library - opengl bindings
if (NOT NO_INCLUDE_GLAD)
  message("MODBUILD - Including GLAD")
  set(GLAD_DIR "${LIB_DIR}/glad")
  add_library("glad" "${GLAD_DIR}/src/glad.c")
  target_include_directories("glad" PRIVATE "${GLAD_DIR}/include")
  target_link_libraries(${PROJECT_NAME} "glad" "${CMAKE_DL_LIBS}")
  target_include_directories(${PROJECT_NAME} PRIVATE "${GLAD_DIR}/include")
else()
  message("MODBUILD - Not Including GLAD")
endif()

# OpenAl - sound
# Alut I just statically link in b/c not widely available on systems
if (NOT NO_INCLUDE_OPENAL)
  # See http://distro.ibiblio.org/rootlinux/rootlinux-ports/more/freealut/freealut-1.1.0/doc/alut.html#CompilingLinking
  # https://stackoverflow.com/questions/15452676/how-to-compile-openal-program-with-g-ubuntu/15452771
  message("MODBUILD - Including OpenAl")
  target_link_libraries(${PROJECT_NAME} "openal")
  target_link_libraries(${PROJECT_NAME} "vorbis")
  target_link_libraries(${PROJECT_NAME} "vorbisfile")

  

  set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
  find_library(ALUT_LIBRARY NAMES alut PATHS /usr/lib /usr/local/lib)
  target_link_libraries(${PROJECT_NAME}  ${ALUT_LIBRARY})
  # In practice this is where it is on my PC 
  # target_link_libraries(${PROJECT_NAME} /usr/lib/x86_64-linux-gnu/libalut.a)
  set(CMAKE_FIND_LIBRARY_SUFFIXES .so .a)

else()
  message("MODBUILD - Not Including OpenAl")
endif()

### Below are static libs only, so we dont have to worry about runtimes
# GLM library - opengl compatible matrix library 
if (NOT NO_INCLUDE_GLM)
  message("MODBUILD - Including GLM")
  set(GLM_DIR "${LIB_DIR}/glm-0.9.9.5")
  set(GLM_TEST_ENABLE OFF CACHE INTERNAL "build glm tests")
  add_subdirectory("${GLM_DIR}")
  target_link_libraries(${PROJECT_NAME} "glm" "${GLM_LIBRARIES}")
  target_include_directories(${PROJECT_NAME} PRIVATE "${GLM_DIR}/include")
else()
  message("MODBUILD - Not Including GLM")
endif()

# STB image library - loading images
if (NOT NO_INCLUDE_STB)
  message("MODBUILD - Including STB")
  set(STB_DIR "${LIB_DIR}/stb")
  add_library("stb" "${STB_DIR}/stb_image.cpp" "${STB_DIR}/stb_image_write.cpp" "${STB_DIR}/stb_image_resize2.cpp")
  target_include_directories("stb" PRIVATE "${STB_DIR}")
  target_link_libraries(${PROJECT_NAME} "stb" "${CMAKE_DL_LIBS}")
  target_include_directories(${PROJECT_NAME} PRIVATE "${STB_DIR}")
else()
  message("MODBUILD - Not Including STB")
endif()


# Command line parsing - CXXopts 
if (NOT NO_INCLUDE_CXXOPTS)
  message("MODBUILD - Including cxxopts")
  set(CXX_DIR "${LIB_DIR}/cxxopts")
  set(CXXOPTS_BUILD_EXAMPLES OFF CACHE INTERNAL "build cxxopts examples")
  set(CXXOPTS_BUILD_TESTS OFF CACHE INTERNAL "build cxxopts tests")
  add_subdirectory("${CXX_DIR}")
  target_link_libraries(${PROJECT_NAME} "cxxopts")
else()
  message("MODBUILD - Not Including cxxopts")
endif()


if (NOT NO_INCLUDE_ASSIMP)
  message("MODBUILD - Including ASSIMP")
  # @todo This can be made faster by settings build options to selectively build importers/etc
  set(ASSIMP_DIR "${LIB_DIR}/assimp")
  set(ASSIMP_BUILD_DIR "${BUILD_LIB_DIR}/assimp")

  set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "If the test suite for Assimp is built in addition to the library.")
  set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE INTERNAL "If the supplementary tools for Assimp are built in addition to the library.")
  
  set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "Disable all exporters" FORCE)
  set(ASSIMP_NO_EXPORT ON CACHE BOOL "Disable all export code" FORCE)
  set(ASSIMP_INSTALL OFF CACHE BOOL "Don't generate install targets" FORCE)

  set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "Disable all importers" FORCE)
  set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "Enable glTF importer" FORCE)
  set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "Enable OBJ importer" FORCE)
  set(ASSIMP_BUILD_COLLADA_IMPORTER ON CACHE BOOL "" FORCE)  # .dae file...lets get rid of this
  set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "Disable sample apps" FORCE)

  add_subdirectory("${ASSIMP_DIR}")
  target_link_libraries(${PROJECT_NAME} "assimp")
  target_include_directories(${PROJECT_NAME} PRIVATE "${ASSIMP_DIR}/include")
  target_include_directories(${PROJECT_NAME} PRIVATE "${ASSIMP_BUILD_DIR}/include")

else()
  message("MODBUILD - Not Including ASSIMP")
endif()


## Bullet - physics -- this should come from the actual lib folder eventually but oh well for now.
if (NOT NO_INCLUDE_BULLET)
  message("MODBUILD - Including Bullet")

  # Options to speed up building by not building everything
  # TODO -- verify how much time these bring down
  # Tried once, build times reduced by  ( ~6m to ~50s / ~90%)
  set(BUILD_CPU_DEMOS OFF CACHE BOOL "")
  set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "")
  set(BUILD_EXTRAS OFF CACHE BOOL "")
  set(BUILD_UNIT_TESTS OFF CACHE BOOL "")
  set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "")
  ####################################################
  add_subdirectory(${LIB_DIR}/bullet3 bullet)
  target_include_directories(${PROJECT_NAME} PRIVATE "${LIB_DIR}/bullet3/src/")
  target_link_libraries(${PROJECT_NAME} "Bullet3Common;BulletDynamics;BulletInverseDynamics;BulletCollision;LinearMath;BulletSoftBody")
else()
  message("MODBUILD - Not Including Bullet")
endif()


## FreeType - font rendering
if (NOT NO_INCLUDE_FREETYPE)
  message("MODBUILD - Including FreeType")
  set(FREETYPE_DIR "${LIB_DIR}/freetype-2.12.1")
  add_subdirectory("${FREETYPE_DIR}")
  target_link_libraries(${PROJECT_NAME} "freetype" "${FREETYPE_DIR}")
  target_include_directories(${PROJECT_NAME} PRIVATE "${FREETYPE_DIR}/include")
else()
  message("MODBUILD - Not Including FreeType")
endif()

## WEBM - video
if (NOT NO_INCLUDE_VIDEO)
  message("MODBUILD - Including WEBM AND VPX")
  #target_link_libraries(${PROJECT_NAME} "webm")
  #target_link_libraries(${PROJECT_NAME} "vpx")

  set(FFMPEG_ROOT "${LIB_DIR}/ffmpeg_n4.4.1/build")
  set(FFMPEG_INCLUDE_DIR "${FFMPEG_ROOT}/include")
  set(FFMPEG_LIB_DIR "${FFMPEG_ROOT}/lib")

  include_directories(${FFMPEG_INCLUDE_DIR})
  link_directories(${FFMPEG_LIB_DIR})

  target_link_libraries(${PROJECT_NAME}
    ${FFMPEG_LIB_DIR}/libavformat.so
    ${FFMPEG_LIB_DIR}/libavcodec.so
    ${FFMPEG_LIB_DIR}/libavutil.so
    ${FFMPEG_LIB_DIR}/libswscale.so
    ${FFMPEG_LIB_DIR}/libswresample.so
  )

else()
  message("MODBUILD - Not Including WEBM AND VPX")
endif()

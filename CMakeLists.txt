#Project Definition 
cmake_minimum_required(VERSION 3.8)
project(modengine)

#Source files
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(BUILD_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib)

#Executable
add_executable(${PROJECT_NAME} 
  "${SRC_DIR}/main.cpp" 
  "${SRC_DIR}/shaders.cpp"
  "${SRC_DIR}/mesh.cpp"
  "${SRC_DIR}/camera.cpp"
  "${SRC_DIR}/sound.cpp"
  "${SRC_DIR}/loadmodel.cpp"
  "${SRC_DIR}/readfont.cpp"
  "${SRC_DIR}/util.cpp"
  "${SRC_DIR}/sprites.cpp"
  "${SRC_DIR}/scene.cpp"
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} "stdc++fs")

# GLFW Library - window management 
set(GLFW_DIR "${LIB_DIR}/glfw-3.3")
set(GLFW_BUILD_EXAMPLES   OFF   CACHE   INTERNAL   "build the glfw example programs")
set(GLFW_BUILD_TESTS      OFF   CACHE   INTERNAL   "build the glfw test programs")
add_subdirectory("${GLFW_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE "GLFW_INCLUDE_NONE")   # makes glfw not include opengl headers.  Think more about this.
target_link_libraries(${PROJECT_NAME} "glfw" "${GLFW_LIBRARIES}")
target_include_directories(${PROJECT_NAME} PRIVATE "${GLFW_DIR}/include") 

# GLAD Library - opengl bindings
set(GLAD_DIR "${LIB_DIR}/glad")
add_library("glad" "${GLAD_DIR}/src/glad.c")
target_include_directories("glad" PRIVATE "${GLAD_DIR}/include")
target_link_libraries(${PROJECT_NAME} "glad" "${CMAKE_DL_LIBS}")
target_include_directories(${PROJECT_NAME} PRIVATE "${GLAD_DIR}/include")

# GLM library - opengl compatible matrix library 
set(GLM_DIR "${LIB_DIR}/glm-0.9.9.5")
set(GLM_TEST_ENABLE OFF CACHE INTERNAL "build glm tests")
add_subdirectory("${GLM_DIR}")
target_link_libraries(${PROJECT_NAME} "glm" "${GLM_LIBRARIES}")
target_include_directories(${PROJECT_NAME} PRIVATE "${GLM_DIR}/include")

# STB image library - loading images
set(STB_DIR "${LIB_DIR}/stb")
add_library("stb" "${STB_DIR}/stb_image.cpp")
target_include_directories("stb" PRIVATE "${STB_DIR}")
target_link_libraries(${PROJECT_NAME} "stb" "${CMAKE_DL_LIBS}")
target_include_directories(${PROJECT_NAME} PRIVATE "${STB_DIR}")

# OpenAl - sound  @todo 
# This was a pain in the ass, probaly doing it wrong. 
# See http://distro.ibiblio.org/rootlinux/rootlinux-ports/more/freealut/freealut-1.1.0/doc/alut.html#CompilingLinking
# https://stackoverflow.com/questions/15452676/how-to-compile-openal-program-with-g-ubuntu/15452771
target_link_libraries(${PROJECT_NAME} "openal")
target_link_libraries(${PROJECT_NAME} "alut")

# Command line parsing - CXXopts 
set(CXX_DIR "${LIB_DIR}/cxxopts")
set(CXXOPTS_BUILD_EXAMPLES OFF CACHE INTERNAL "build cxxopts examples")
set(CXXOPTS_BUILD_TESTS OFF CACHE INTERNAL "build cxxopts tests")
add_subdirectory("${CXX_DIR}")
target_link_libraries(${PROJECT_NAME} "cxxopts")

# ASSIMP
# @todo This can be made faster by settings build options to selectively build importers/etc
set(ASSIMP_DIR "${LIB_DIR}/assimp")
set(ASSIMP_BUILD_DIR "${BUILD_LIB_DIR}/assimp")
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "If the test suite for Assimp is built in addition to the library.")
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE INTERNAL "If the supplementary tools for Assimp are built in addition to the library.")
add_subdirectory("${ASSIMP_DIR}")
target_link_libraries(${PROJECT_NAME} "assimp")
target_include_directories(${PROJECT_NAME} PRIVATE "${ASSIMP_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${ASSIMP_BUILD_DIR}/include")

# Bullet - physics @todo

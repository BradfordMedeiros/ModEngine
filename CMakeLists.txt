#Project Definition 
cmake_minimum_required(VERSION 3.8)
project(modengine)

#Source files
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")

#Executable
add_executable(${PROJECT_NAME} 
  "${SRC_DIR}/main.cpp" 
  "${SRC_DIR}/shaders.cpp"
  "${SRC_DIR}/mesh.cpp"
  "${SRC_DIR}/options.cpp"
)

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

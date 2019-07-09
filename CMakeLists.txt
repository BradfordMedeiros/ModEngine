#Project Definition 
cmake_minimum_required(VERSION 3.1)
project(modengine)

#Source files
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

#Executable 
add_executable(${PROJECT_NAME} "${SRC_DIR}/main.cpp")

#####################################################################
# Powderpunk_Server Project
#####################################################################
cmake_minimum_required(VERSION 3.17)

# Project Name
project(
    FreenectGreenScreen VERSION 0.1.0
    DESCRIPTION "Green screen for your kinect!"
    LANGUAGES CXX
)

#####################################################################
# Compiling source code
#####################################################################

# Generate JSON file for LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output binary files in specific dir
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Generate binary
add_executable(frennect-green-screen
    src/main.cpp
)

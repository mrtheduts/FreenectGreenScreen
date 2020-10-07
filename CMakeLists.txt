#####################################################################
# Freenect Green Screen Project
#####################################################################
cmake_minimum_required(VERSION 3.17)

# Project Name
project(
    FreenectGreenScreen VERSION 0.2.1
    DESCRIPTION "Green screen for your kinect!"
    LANGUAGES CXX
)

# Generate JSON file for LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output binary files in specific dir
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#####################################################################
# Compiling source code
#####################################################################

# Search Packages
find_package(OpenCV COMPONENTS core imgproc highgui REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
find_package(Threads REQUIRED)
find_package(libfreenect REQUIRED)

# Necessary for libfreenect
include_directories("/usr/include/libusb-1.0/")

# Generate binary
add_executable(freenect-green-screen
    src/main.cpp
    src/CmdLineOpts.cpp
    src/MyFreenectDevice.cpp
    src/utils.cpp
)

target_link_libraries(freenect-green-screen
    ${OpenCV_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
    ${FREENECT_LIBRARIES}
)

target_compile_options(freenect-green-screen PRIVATE -Werror -Wall -Wextra)

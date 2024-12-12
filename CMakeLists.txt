# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_COMPILER "/usr/bin/g++")

# Project name and version
project(MyGTKApp VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find GTKMM, CURL, JSON packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)
find_package(CURL REQUIRED)

## Generate resources.c if it doesn't exist
#add_custom_command(
#    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/resources.c
#    COMMAND glib-compile-resources --generate-source
#            --target=${CMAKE_CURRENT_SOURCE_DIR}/src/resources.c
#            ${CMAKE_CURRENT_SOURCE_DIR}/src/styles.gresource.xml
#    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/styles.gresource.xml
#)

# Include directories
include_directories(${GTKMM_INCLUDE_DIRS})
link_directories(${GTKMM_LIBRARY_DIRS})

# Compiler definitions and options
add_definitions(${GTKMM_CFLAGS_OTHER})

# Source files
#set(SOURCES src/main.cpp src/resources.c)
set(SOURCES src/main.cpp)

# Add the executable
add_executable(MyGTKApp ${SOURCES})

# Link GTKMM libraries
target_link_libraries(MyGTKApp ${GTKMM_LIBRARIES} ${CURL_LIBRARIES})
target_include_directories(MyGTKApp PRIVATE ${GTKMM_INCLUDE_DIRS})
target_compile_options(MyGTKApp PRIVATE ${GTKMM_CFLAGS_OTHER})

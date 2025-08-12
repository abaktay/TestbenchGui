cmake_minimum_required(VERSION 3.16)
project(qt-testbench)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Enable Qt MOC (Meta-Object Compiler)
set(CMAKE_AUTOMOC ON)

# Collect all source files from src/ directory
file(GLOB SOURCES
    "src/*.cpp"
    "src/*.c"
)

# Collect all header files from inc/ directory
file(GLOB HEADERS
    "inc/*.hpp"
    "inc/*.h"
)

# Create executable FIRST
add_executable(qt-testbench ${SOURCES})

# THEN add include directories (after target exists)
target_include_directories(qt-testbench PRIVATE inc/)

# Link Qt6 libraries
target_link_libraries(qt-testbench Qt6::Core Qt6::Widgets)


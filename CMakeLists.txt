cmake_minimum_required(VERSION 3.16)
project(FruitFlyBrainSimulator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build options
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_VISUALIZATION "Build visualization tools" OFF)
option(USE_OPENMP "Enable OpenMP parallelization" ON)
option(USE_MPI "Enable MPI distributed computing" OFF)

# Find dependencies
find_package(Threads REQUIRED)

if(USE_OPENMP)
    find_package(OpenMP)
    if(OpenMP_CXX_FOUND)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    endif()
endif()

if(USE_MPI)
    find_package(MPI REQUIRED)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Main library sources
set(LIB_SOURCES
    src/neuron.cpp
    src/synapse.cpp
    src/brain_region.cpp
    src/connectome.cpp
    src/simulator.cpp
    src/io_utils.cpp
    src/visualization.cpp
)

# Create static library
add_library(brain_sim STATIC ${LIB_SOURCES})
target_link_libraries(brain_sim Threads::Threads)

if(OpenMP_CXX_FOUND)
    target_link_libraries(brain_sim OpenMP::OpenMP_CXX)
endif()

# Main executable
add_executable(brain_app src/main.cpp)
target_link_libraries(brain_app brain_sim)

# Data loader executable
add_executable(data_loader src/data_loader_main.cpp)
target_link_libraries(data_loader brain_sim)

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_executable(run_tests tests/run_tests.cpp)
    target_link_libraries(run_tests brain_sim)
    add_test(NAME UnitTests COMMAND run_tests)
endif()

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(brain_sim PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(brain_app PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Installation
install(TARGETS brain_app data_loader DESTINATION bin)
install(DIRECTORY data/ DESTINATION share/brain_app/data)

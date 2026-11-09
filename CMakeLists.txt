cmake_minimum_required(VERSION 3.22)
project(heartbeat_radar_sensor_native LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

enable_testing()

include_directories(src)

file(GLOB_RECURSE CORE_SOURCES
    "src/core/*.cpp"
    "src/adapters/buzzers/*.cpp"
    "src/adapters/sensors/*.cpp"
)
if(CORE_SOURCES)
    add_library(hrs_core STATIC ${CORE_SOURCES})
    target_include_directories(hrs_core PUBLIC src)
endif()

add_subdirectory(test_native)

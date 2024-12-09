# Minimum CMake version required
cmake_minimum_required(VERSION 3.22.1)

# Project name
project("pitch")

# Ensure proper C++ standard is used
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required package Oboe via Prefab
find_package(oboe REQUIRED CONFIG)

# Source files
set(SOURCES
        pitch.cpp
        pitch_detector.cpp
        pyin.cpp
)

# Add shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link required libraries
target_link_libraries(${PROJECT_NAME}
        oboe::oboe
        android
        log
)

# 添加优化编译选项
if(ANDROID_ABI MATCHES "arm64-v8a")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math -ftree-vectorize")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a+simd")
    
    # 添加 NEON 相关定义
    add_definitions(-D__ARM_NEON)
    add_definitions(-D__aarch64__)
endif()

# 添加 NEON 支持
if(ANDROID_ABI MATCHES "arm64-v8a")
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_NEON)
endif()

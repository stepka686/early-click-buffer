
cmake_minimum_required(VERSION 3.25)

project(EarlyClickBuffer VERSION 1.0.0)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

setup_geode_mod(${PROJECT_NAME})

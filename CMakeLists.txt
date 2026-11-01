cmake_minimum_required(VERSION 3.20)

project(Minecraft2D VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(vendor/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendor/SDL_image EXCLUDE_FROM_ALL)

add_executable(Minecraft2D src/main.cpp)

target_link_libraries(Minecraft2D PRIVATE SDL3::SDL3 SDL3_image::SDL3_image)
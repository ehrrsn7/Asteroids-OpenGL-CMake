cmake_minimum_required(VERSION 3.16)
project(Asteroids LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(Asteroids
  main.cpp
  src/window.cpp
  src/glad/glad.c
)

target_include_directories(Asteroids PRIVATE include)
target_link_libraries(Asteroids PRIVATE OpenGL::GL glfw)

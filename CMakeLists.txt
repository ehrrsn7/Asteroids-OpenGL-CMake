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

# Automatically copy vcpkg DLLs to the output directory on Windows
if(WIN32)
  add_custom_command(TARGET Asteroids POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_FILE_DIR:glfw3>/glfw3.dll"
    "$<TARGET_FILE_DIR:Asteroids>"
    VERBATIM
  )
endif()

target_include_directories(Asteroids PRIVATE include)
target_link_libraries(Asteroids PRIVATE OpenGL::GL glfw)

cmake_minimum_required(VERSION 3.16)
project(Asteroids LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required system packages (adjust if using SDL2/SFML instead of GLFW)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED) # Replace with find_package(SDL2 REQUIRED) if using SDL

# Gather source files
set(SOURCES
   main.cpp
   src/window.cpp
   src/game.cpp
   src/glad/glad.c
)

# Define target executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Specify include directories
# Using PRIVATE target_include_directories is preferred over global include_directories()
target_include_directories(${PROJECT_NAME} PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
   OpenGL::GL
   glfw # Or SDL2::SDL2 / ${GLFW_LIBRARIES} depending on your setup
)

# Copy assets folder to the build directory so relative paths work seamlessly
add_custom_command(
   TARGET ${PROJECT_NAME} POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy_directory
   ${CMAKE_CURRENT_SOURCE_DIR}/assets
   $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)

cmake_minimum_required(VERSION 3.16)
project(Asteroids LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

if(WIN32)
   add_custom_command(
      TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "C:/msys64/ucrt64/bin/glfw3.dll"
      $<TARGET_FILE_DIR:${PROJECT_NAME}>/glfw3.dll
   )

   if(CMAKE_CXX_COMPILER)
      get_filename_component(MINGW_BIN_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
      set(MINGW_RUNTIME_DLLS
         "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
         "${MINGW_BIN_DIR}/libstdc++-6.dll"
         "${MINGW_BIN_DIR}/libwinpthread-1.dll"
      )

      foreach(DLL ${MINGW_RUNTIME_DLLS})
         if(EXISTS ${DLL})
            add_custom_command(
               TARGET ${PROJECT_NAME} POST_BUILD
               COMMAND ${CMAKE_COMMAND} -E copy_if_different
               "${DLL}"
               $<TARGET_FILE_DIR:${PROJECT_NAME}>
            )
         endif()
      endforeach()
   endif()
endif()

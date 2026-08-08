cmake_minimum_required(VERSION 3.16)
project(Asteroids LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)

find_package(OpenGL REQUIRED)
find_package(glfw3 QUIET)

include(FetchContent)
set(DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

set(GLFW_ZIP_ARCHIVE "${CMAKE_CURRENT_SOURCE_DIR}/assets/package/glfw-3.5.1.zip")
if(EXISTS "${GLFW_ZIP_ARCHIVE}")
   FetchContent_Declare(
      glfw
      URL "${GLFW_ZIP_ARCHIVE}"
   )

   set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
   set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
   set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

   FetchContent_MakeAvailable(glfw)
else()
   message(FATAL_ERROR "GLFW was not found by find_package(glfw3), and the offline archive ${GLFW_ZIP_ARCHIVE} is missing.\nDownload the GLFW source zip from https://glfw.org/download.html and place it in assets/package/glfw-3.5.1.zip.")
endif()

# ---------------------------------------------------------
# SOURCES
# ---------------------------------------------------------
file(GLOB TOP_LEVEL_SOURCES CONFIGURE_DEPENDS "*.cpp")
file(GLOB_RECURSE SRC_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.c")

add_executable(Asteroids
   ${TOP_LEVEL_SOURCES}
   ${SRC_SOURCES}
)

target_include_directories(Asteroids PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link everything together
target_link_libraries(Asteroids PRIVATE
   glfw
   OpenGL::GL
)

cmake_minimum_required(VERSION 3.6)
project(solar_system)

set(CMAKE_CXX_STANDARD 11)


add_subdirectory(glm EXCLUDE_FROM_ALL)
add_subdirectory(glew/build/cmake EXCLUDE_FROM_ALL)
add_subdirectory(glfw EXCLUDE_FROM_ALL)

# Find required libraries
#find_package(OpenGL REQUIRED)

# Set include directories
include_directories(
    "glm/"
	"glew/include/"
	"gfw/include"
    .
)

link_directories("glew")

# Set source files
set(SOURCE_FILES solar_system.cpp)

# Add executable target
add_executable(solar_system ${SOURCE_FILES})

# Link libraries
target_link_libraries(solar_system glew_s glfw ${CMAKE_THREAD_LIBS_INIT})
cmake_minimum_required(VERSION 3.6)
project(solar_system)

set(CMAKE_CXX_STANDARD 11)

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(glm EXCLUDE_FROM_ALL)
add_subdirectory(glew/build/cmake EXCLUDE_FROM_ALL)
add_subdirectory(SDL EXCLUDE_FROM_ALL)
add_subdirectory(SDL_ttf EXCLUDE_FROM_ALL)

# Find required libraries
#find_package(OpenGL REQUIRED)

# Set include directories
include_directories(
    "glm/"
	"glew/include/"
	"SDL/include/"
	"SDL_ttf/"
    .
)

link_directories("glew")

# Set source files
set(SOURCE_FILES solar_system.cpp)

# Add executable target
add_executable(solar_system ${SOURCE_FILES})

# Link libraries
target_link_libraries(solar_system glew_s SDL2-static SDL2_ttf ${CMAKE_THREAD_LIBS_INIT})
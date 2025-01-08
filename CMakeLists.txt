cmake_minimum_required(VERSION 3.17)
project(SolarSystem)

set(CMAKE_CXX_STANDARD 11)

# Find required libraries
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)
find_package(glm REQUIRED) 

# Set include directories
include_directories(
    ${OpenGL_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
    ${glm_INCLUDE_DIRS}
    .
)

# Set source files
set(SOURCE_FILES solar_system.cpp)

# Add executable target
add_executable(SolarSystem ${SOURCE_FILES})

# Link libraries
target_link_libraries(SolarSystem ${OpenGL_LIBRARIES} ${GLEW_LIBRARIES} ${GLUT_LIBRARIES} ${glm_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
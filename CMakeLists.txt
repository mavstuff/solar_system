cmake_minimum_required(VERSION 3.17)
project(SolarSystem)

set(CMAKE_CXX_STANDARD 11)


add_subdirectory(glm EXCLUDE_FROM_ALL)
add_subdirectory(glew/build/cmake EXCLUDE_FROM_ALL)
add_subdirectory(freeglut EXCLUDE_FROM_ALL)



# Find required libraries
find_package(OpenGL REQUIRED)

# Set include directories
include_directories(
    ${OpenGL_INCLUDE_DIRS}
    "glm/"
	"glew/include/"
    "freeglut/include/"
    .
)

link_directories("glew")

# Set source files
set(SOURCE_FILES solar_system.cpp)

# Add executable target
add_executable(SolarSystem ${SOURCE_FILES})


# Link libraries
target_link_libraries(SolarSystem ${OpenGL_LIBRARIES} freeglut_static glew_s ${CMAKE_THREAD_LIBS_INIT})
cmake_minimum_required(VERSION 3.10)
project(ParticleSimulation)

set(CMAKE_CXX_STANDARD 17)

# Add GLFW
find_package(glfw3 3.3 REQUIRED)

# Include GLAD and KHR
include_directories(include)

# Add the executable
add_executable(ParticleSimulation src/main.cpp src/glad.c)

# Link GLFW
target_link_libraries(ParticleSimulation glfw)

# Link OpenGL framework
target_link_libraries(ParticleSimulation "-framework OpenGL")

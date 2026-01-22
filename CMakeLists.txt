cmake_minimum_required(VERSION 3.20)

project(CaveXRT)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(GL/glfw)
add_subdirectory(GL/glad)
add_subdirectory(GL/glm)

add_executable(CaveXRT src/main.cpp)

target_link_libraries(CaveXRT glfw glad glm)

target_include_directories(
    CaveXRT PUBLIC
    GL/glad/include
    GL/glfw/include
    GL/glm
)
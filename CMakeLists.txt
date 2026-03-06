cmake_minimum_required(VERSION 3.20)

project(CaveXRT)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(GL/glfw)
add_subdirectory(GL/glad)
add_subdirectory(GL/glm)
add_subdirectory(GL/assimp)
add_subdirectory(GL/json)

add_executable(CaveXRT 
src/main.cpp
src/Shader.h 
src/Shaders.cpp
src/Mesh.h
src/Mesh.cpp
src/ModelLoader.h
src/ModelLoader.cpp
src/CaveXRTConfig.h
src/CaveXRTConfig.cpp
GL/lodepng/lodepng.cpp 
src/RenderTarget.h
src/RenderTarget.cpp 
src/Skybox.h 
src/DepthTarget.h
src/DepthTarget.cpp)

target_link_libraries(CaveXRT glfw glad glm assimp)

target_include_directories(
    CaveXRT PUBLIC
    GL/glad/include
    GL/glfw/include
    GL/glm
    GL/assimp/include
    GL/lodepng
    GL/json/include
)
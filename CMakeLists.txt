cmake_minimum_required(VERSION 3.20)

project(CaveXRT)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/GL/imgui)

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
src/CaveCompute.h
src/CaveCompute.cpp 
src/ParticleSystem.h
src/ParticleSystem.cpp
src/FluidRenderTarget.h
src/FluidRenderTarget.cpp
src/VideoRecorder.h
src/VideoRecorder.cpp
${IMGUI_DIR}/imgui.cpp
${IMGUI_DIR}/imgui_draw.cpp
${IMGUI_DIR}/imgui_tables.cpp
${IMGUI_DIR}/imgui_widgets.cpp
${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)
 
target_link_libraries(CaveXRT glfw glad glm assimp)

target_include_directories(
    CaveXRT PUBLIC
    GL/glad/include
    GL/glfw/include
    GL/glm
    GL/assimp/include
    GL/lodepng
    GL/json/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_compile_definitions(CaveXRT PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)
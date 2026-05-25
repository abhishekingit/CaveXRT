# CaveXRT

https://github.com/user-attachments/assets/ec95fbb7-0b11-40c8-8464-c8cff422f08b

CaveXRT (Compute Accelerated Visualization Engine) is a real-time GPU fluid simulation engine focused on comparing modern particle-based fluid simulation methods and rendering techniques. The engine implements both Smoothed Particle Hydrodynamics (SPH) and Position Based Fluids (PBF) entirely on the GPU using compute shaders, enabling interactive fluid simulation and visualization in real time.

To improve visual realism, the simulation is rendered using a screen-space fluid rendering pipeline with depth/thickness reconstruction and a Narrow Range Filter for smooth surface generation. The engine also includes tools for simulation recording and particle export, allowing simulations to be captured and integrated into external rendering and cinematic workflows.

https://github.com/user-attachments/assets/06bf853e-97fc-456f-b284-f48bab1f731b

Cinematic fluid sequence created using CaveXRT simulations, Blender surface reconstruction, and Unreal Engine 5.6 rendering pipeline.

CaveXRT features GPU-accelerated particle simulation with uniform grid neighbor search, real-time SPH and PBF solvers, vorticity confinement, XSPH viscosity, and screen-space surface reconstruction techniques for smooth fluid rendering. The rendering pipeline includes depth and thickness texture generation, narrow range filtering, reconstructed surface normals, and fluid shading for visually continuous fluid surfaces. The engine additionally supports particle cache serialization/export, interactive simulation controls, and simulation screen recording for debugging, visualization, and cinematic workflows in external tools such as Blender and Unreal Engine. The engine is built using C++, OpenGL, and GLSL Compute Shaders for real-time GPU-accelerated simulation and rendering.

## Project Goal

The goal of CaveXRT is to explore the trade-offs between SPH and PBF methods in terms of stability, incompressibility, and real-time performance while building a modern GPU-driven fluid rendering pipeline. The engine additionally explores how real-time simulation data can be integrated into external rendering and cinematic pipelines through particle export and offline visualization workflows.
 
### Requirements
- CMake >= 3.20
- C++20 compiler (MSVC/Clang/GCC)
- Ninja build system (recommended)
- OpenGL 4.3 capable GPU and drivers

### Third-party libraries
Submodules in `GL/` and built via `add_subdirectory`:
- GLFW (windowing/input)
- GLAD (OpenGL loader)
- GLM (math)
- Assimp (model loading)

### Build (Windows, Visual Studio)
1. Configure: `cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release`
2. Build: `cmake --build build`
3. Run: `build\CaveXRT.exe`

### Project Structure
- `CMakeLists.txt` - build configuration
- `src/` - source code
- `src/Shaders/` - GLSL shader files
- `assets/models/` - Model files
- `CaveXRTConfig.json` - render configurations (camera, lights)


`CaveXRTConfig.json` allows Hot reloading of shaders and dynamic updates to camera and light settings without recompiling the application. Use the `R` key to reload the render settings at runtime.

## CaveXRT

## GPU Fluid Simulation using Compute Shaders 

### Simple Particle system using Compute Shaders


Simple particle system using compute shaders. Particles are initialized with random positions and velocities and updated each frame based on delta time. Here the particles are rendered as points with a basic shader that colors them. 
Here the Particle system doesn't support collisions/hitboxes yet which is why it passes through the quad. 

### Requirements
- CMake >= 3.20
- C++20 compiler (MSVC/Clang/GCC)
- Ninja build system (recommended)
- OpenGL 3.3 capable GPU and drivers

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
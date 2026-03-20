## CaveXRT

## GPU Fluid Simulation using Compute Shaders 

### Simple Particle system using Compute Shaders

https://github.com/user-attachments/assets/edb9c679-09b3-4ae1-a8b5-c431c7f1fe30

### Bounding boxes and collisions 

https://github.com/user-attachments/assets/57ba525c-6b77-4b64-906f-2c669de891b1

### Uniform Grid for neighbour search

https://github.com/user-attachments/assets/473096e1-6860-4bd3-9c70-4987ac14bb954

Simple particle system using compute shaders. Particles are initialized with random positions and velocities and updated each frame based on delta time. Here the particles are rendered as points with a basic shader that colors them. Particle systems support collisions using bounding boxes and also theres a damping factor added to the velocity computation. The Uniform grid needed for performing neighbour search for SPH is visualized using the particles being arranged according to the grid and also colored according to the Cell IDs. The uniform grid operations like counting the particles and sorting are also performed using compute shaders. The grid helps in performing the simulation passes for density, viscosity and pressure efficiently. 

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

## CaveXRT

## GPU Fluid Simulation using Compute Shaders 

### Simple Particle system using Compute Shaders

https://github.com/user-attachments/assets/edb9c679-09b3-4ae1-a8b5-c431c7f1fe30

### Bounding boxes and collisions 

https://github.com/user-attachments/assets/57ba525c-6b77-4b64-906f-2c669de891b1

### Uniform Grid for neighbour search

https://github.com/user-attachments/assets/741e9e45-6e94-48e7-8ec9-a4de1ea8fe95

### Density pass with gravity force

https://github.com/user-attachments/assets/a244efc1-2b6a-430a-bb62-12a78f3cfbe0

Simple particle system using compute shaders. Particles are initialized along the uniform grid and positions and velocities are updated each frame based on delta time. Here the particles are rendered as points with a basic shader that colors them. Particle systems support collisions using bounding boxes and also theres a damping factor added to the velocity computation. The Uniform grid needed for performing neighbour search for SPH is visualized using the particles being arranged according to the grid and also colored according to the Cell IDs. The uniform grid operations like counting the particles and sorting are also performed using compute shaders. The grid helps in performing the simulation passes for density, viscosity and pressure efficiently. 

The density pass is the first SPH pass where the cubic kernel is used for smoothing and the uniform grid helps in computation of density for each particle based on neighbours. For visualizing I am coloring the particles based on density. The density pass is essential for other passes involving viscosity and pressure solvers. The smoothing radius here is chosen as the grid cell size which is particle sim radius * 4 which captures a good amount of neighbours for each particle. The gravity force is added as acceleration for the integrator. 

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

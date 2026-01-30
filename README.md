## CaveXRT

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


### Mesh processing and Model loading 

<img alt="UtahTeapot.gif" src="https://github.com/abhishekingit/CaveXRT/blob/main/results/modelload.gif?raw=true" data-hpc="true" class="Box-sc-g0xbh4-0 kzRgrI" height="512px">

A simple teapot model, Shaders can be recompiled using F6 key. 
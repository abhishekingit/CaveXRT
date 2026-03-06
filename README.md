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
- `CaveXRTConfig.json` - render configurations (camera, lights)


`CaveXRTConfig.json` allows Hot reloading of shaders and dynamic updates to camera and light settings without recompiling the application. Use the `R` key to reload the render settings at runtime.


### Mesh processing and Model loading 

<img alt="UtahTeapot.gif" src="https://github.com/abhishekingit/CaveXRT/blob/main/results/modelload.gif?raw=true" data-hpc="true" class="Box-sc-g0xbh4-0 kzRgrI" height="512px">

A simple teapot model, Shaders can be recompiled using F6 key. 


### Blinn Shading and lighting 

<img alt="blinnshade.gif" src="https://github.com/abhishekingit/CaveXRT/blob/main/results/blinnshade.gif?raw=true" data-hpc="true" class="Box-sc-g0xbh4-0 kzRgrI" height="512px">

Blinn shading done in view space with movable light source.

<img alt="rimlight.gif" src="https://github.com/abhishekingit/CaveXRT/blob/main/results/rimlight.gif?raw=true" data-hpc="true" class="Box-sc-g0xbh4-0 kzRgrI" height="512px">

Rim lighting added on top of Blinn shading


### Textures and Multiple materials

<img alt="yodaMultipleMaterials" src="https://github.com/abhishekingit/CaveXRT/blob/main/results/yodaMultipleMt.gif?raw=true" data-hpc="true" class="Box-sc-g0xbh4-0 kzRgrI" height="512px">

Yoda model with multiple materials and textures. Currently diffuse, specular and normals map can be loaded but the shader does not support normal mapping yet.


### Render To Texture 

<img alt="yodaRenderTexture" src="https://github.com/abhishekingit/CaveXRT/blob/main/results/yodaRenderTexture.gif?raw=true" data-hpc="true" class="Box-sc-g0xbh4-0 kzRgrI" height="512px">

Yoda model rendered as a texture on a quad plane. Using the custom RenderTarget class to create framebuffer objects and attach textures using depth buffer.


### Reflections and Environment mapping

<img alt="yodaReflection" src="https://github.com/abhishekingit/CaveXRT/blob/main/results/yodaRender.gif?raw=true" data-hpc="true" class="Box-sc-g0xbh4-0 kzRgrI" height="720px">

Reflections with environment mapping using cubemaps. The fragment shader samples the cubemap texture based on the reflection vector calculated from the view direction and surface normal. These reflections are still incorrect which you can notice on the spout and lid, and also the bottom of the teapot has some grazing reflections happening. 
Theres support for skybox added using cubemaps.

### Shadow Mapping

<img alt="shadows" src="https://github.com/abhishekingit/CaveXRT/blob/main/results/shadows.gif?raw=true" data-hpc="true" class="Box-sc-g0xbh4-0 kzRgrI" height="512px">

Shadow mapping implemented using depth maps. The scene is rendered from the light's perspective to create a depth map, which is then used in the main render pass to determine if fragments are in shadow. The shadows are hard-edged and there are some artifacts due to the lack of biasing and filtering, but it demonstrates basic shadow mapping.

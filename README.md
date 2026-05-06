# Dynamic 3D Scene — OpenGL

An interactive 3D scene written in modern **C++ / OpenGL** as a Computer Graphics course project. The scene combines custom GLSL shaders, a free-fly camera, dynamic lighting and a skybox to render a small outdoor environment populated with imported `.obj` models (trees, rocks, flowers, etc.).

## Features

- Free-fly **camera** with mouse look + WASD movement
- Custom **GLSL shaders** (`shaders/basic.*`, `shaders/skybox.*`, `shaders/shadow.*`) for Phong-style lighting and shadow mapping
- **Cubemap skybox** (six textures under `textures/skybox/`)
- **Dynamic shadows** rendered via a depth-pass + shadow shader
- **OBJ model loading** via `tiny_obj_loader.h` — the scene currently uses tree, rock and flower models from the `models/` folder
- Texture loading through `stb_image.h`
- Modular C++ design: `Camera`, `Window`, `Shader`, `Mesh`, `Model3D` each in its own translation unit

## Project layout

```
ProiectPG.sln                  # Visual Studio solution
main.cpp                       # Scene setup and render loop
Camera.{cpp,hpp}               # First-person camera
Window.{cpp,h}                 # GLFW window wrapper
Shader.{cpp,hpp}               # GLSL shader program loader
Mesh.{cpp,hpp}                 # Vertex/index buffers + draw call
Model3D.{cpp,hpp}              # Loads and draws a multi-mesh OBJ model
shaders/                       # basic, shadow, skybox shaders (.vert + .frag)
textures/skybox/               # Six cubemap faces
models/                        # OBJ + MTL + PNG assets (tree, rock, flower, ...)
stb_image.{h,cpp}              # Public-domain image loader
tiny_obj_loader.{h,cpp}        # Public-domain OBJ loader
launch.bat                     # Convenience launcher (Windows)
```

## Dependencies

- OpenGL 3.3+
- GLEW
- GLFW
- GLM
- (bundled) stb_image, tiny_obj_loader

The included Visual Studio project (`ProiectPG.vcxproj`) expects these libraries to be available via your usual OpenGL setup (vcpkg, NuGet, or manual link directories).

## Build

### Windows — Visual Studio
Open `ProiectPG.sln`, choose `x64 / Release`, and **Build → Build Solution**. Then run via `launch.bat` or the IDE.

### Linux / macOS (manual)
A typical command line, once GLEW / GLFW / GLM are installed system-wide:

```bash
g++ -std=c++17 main.cpp Camera.cpp Window.cpp Shader.cpp Mesh.cpp Model3D.cpp \
    stb_image.cpp tiny_obj_loader.cpp \
    -lGL -lGLEW -lglfw -o scene
./scene
```

## Controls

- `W` / `A` / `S` / `D` — move
- Mouse — look around
- `Esc` — quit

(See `main.cpp` for any extra keys wired to scene-specific actions, such as toggling lights or animations.)

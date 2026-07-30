# Shader Sketchbook

Shader Sketchbook is a compact C++ and OpenGL playground for signed-distance-field scenes.

It shows shader math as a live, adjustable image. It also captures the same scene with a deterministic CPU renderer.

## Value

- Explore smooth unions, repetition, ray marching, normals, lighting, and tone mapping.
- Edit GLSL while the demo runs. The renderer keeps the last valid program after a failed reload.
- Compare a committed PPM capture during every test run.
- Build with CMake and the platform OpenGL library. No runtime package or secret is required.

## Release 0.1

The first release renders one configurable SDF scene.

The scene combines a sphere, a box, repeated satellites, and a tiled ground plane.

The window title reports live parameters and shader status.

## Architecture

```text
Win32 window
    -> WGL context
    -> OpenGL renderer
    -> GLSL vertex and fragment programs

Portable SDF core
    -> signed-distance queries
    -> CPU reference renderer
    -> PPM comparison fixture
```

| Area | Location | Responsibility |
| --- | --- | --- |
| Scene model | `include/shader_sketchbook/sdf_scene.hpp` | Public settings and SDF operations |
| Reference render | `src/core/sdf_scene.cpp` | Deterministic CPU ray marcher |
| Image utility | `src/core/image.cpp` | PPM load, save, and error measurement |
| Native host | `src/demo/native_window.cpp` | Win32 window and keyboard input |
| GPU host | `src/demo/opengl_renderer.cpp` | WGL context, uniforms, and presentation |
| Shader reload | `src/demo/shader_program.cpp` | Compile, link, watch, and preserve GLSL programs |
| GLSL scene | `shaders/scene.frag` | GPU SDF scene and lighting |
| Fixtures | `tests/fixtures/` | Committed visual regression capture |

## Setup

The interactive demo targets Windows with a desktop OpenGL driver.

Run these commands from the repository root.

```powershell
cmake -S . -B build-vs -G "Visual Studio 16 2019" -A x64
cmake --build build-vs --config Release --parallel
ctest --test-dir build-vs -C Release --output-on-failure
.\build-vs\Release\shader_sketchbook.exe
```

The project has no third-party runtime dependency.

The test targets also build on Linux and macOS.

Non-Windows builds provide the portable tests and a short platform notice.

## Controls

| Key | Change |
| --- | --- |
| `A` / `D` | Lower or raise shape blend |
| `S` / `W` | Lower or raise sphere radius |
| `E` / `Q` | Lower or raise repeat spacing |
| `Z` / `X` | Lower or raise light energy |
| `R` | Force shader reload |
| `Space` | Pause or resume animation |
| `Esc` | Close the demo |

Save a valid edit in `shaders/scene.frag` or `shaders/scene.vert`.

The demo detects the file timestamp and reloads the program on the next frame.

## Sample output

```text
sdf_core: PASS
default_checksum: 5669664343924647534
reference_image: PASS
checksum: 5669664343924647534
```

The checksum identifies the 32 by 18 default capture.

The fixture test fails when the render exceeds its cross-platform tolerance.

## Tests

`sketchbook_sdf_tests` checks field values, dimensions, checksums, and parameter sensitivity.

`sketchbook_reference_tests` compares the default render with `tests/fixtures/sdf_scene_reference.ppm`.

Run all registered tests with `ctest --test-dir build-vs -C Release --output-on-failure`.

CI builds on Windows and Ubuntu.

Current status: two deterministic tests pass locally with Visual Studio 2019 and CMake 4.3.1.

## Limitations

- The interactive host currently uses Win32 and WGL.
- The reference image checks the CPU renderer, not a GPU framebuffer.
- The demo uses a compatibility OpenGL context for wide driver support.
- Shader reload watches file timestamps, so some editors need a saved file before reload.
- The reference capture uses a small image for fast CI feedback.

## Roadmap

- 0.2: Add a shader exercise browser with named GLSL lessons.
- 0.3: Add framebuffer capture and GPU image comparison.
- 0.4: Add camera orbit controls and signed-distance 2D sketches.
- 0.5: Add a cross-platform window layer with an optional tested backend.

## License

MIT. See [LICENSE](LICENSE).


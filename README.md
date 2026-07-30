# Shader Sketchbook

Shader Sketchbook is a compact C++ and OpenGL playground for signed-distance-field scenes.

It shows shader math as a live, adjustable image. It also captures the same scene with a deterministic CPU renderer.

## Value

- Explore smooth unions, repetition, ray marching, normals, lighting, and tone mapping.
- Browse three named GLSL lessons from the demo with keyboard navigation.
- Edit GLSL while the demo runs. The renderer keeps the last valid program after a failed reload.
- Compare a committed PPM capture during every test run.
- Build with CMake and the platform OpenGL library. No runtime package or secret is required.

## Release 0.2

This release adds a keyboard-driven shader exercise browser.

Each lesson has a name, summary, learning focus, and fragment shader path.

The renderer switches lessons without restarting the window.

The default lesson combines a sphere, a box, repeated satellites, and a tiled ground plane.

The window title reports the active lesson, live parameters, and shader status.

| Lesson | Focus |
| --- | --- |
| SDF Fusion | Smooth unions and distance fields |
| Normals and Lighting | Finite differences and responsive shading |
| Space Repetition | Modulo repetition and field composition |

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

Lesson catalog
    -> named lesson metadata
    -> wrapped keyboard navigation
    -> fragment shader selection
```

| Area | Location | Responsibility |
| --- | --- | --- |
| Scene model | `include/shader_sketchbook/sdf_scene.hpp` | Public settings and SDF operations |
| Lesson catalog | `include/shader_sketchbook/lesson_catalog.hpp` | Lesson metadata and navigation |
| Reference render | `src/core/sdf_scene.cpp` | Deterministic CPU ray marcher |
| Image utility | `src/core/image.cpp` | PPM load, save, and error measurement |
| Native host | `src/demo/native_window.cpp` | Win32 window and keyboard input |
| GPU host | `src/demo/opengl_renderer.cpp` | WGL context, uniforms, and presentation |
| Shader reload | `src/demo/shader_program.cpp` | Compile, link, watch, and preserve GLSL programs |
| GLSL scene | `shaders/scene.frag` | GPU SDF scene and lighting |
| Lesson shaders | `shaders/lessons/` | Focused GLSL exercises for normals and repetition |
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
| `N` / `P` or arrow keys | Next or previous GLSL lesson |
| `1` / `2` / `3` | Open a lesson directly |
| `Space` | Pause or resume animation |
| `Esc` | Close the demo |

Save a valid edit in `shaders/scene.frag` or `shaders/scene.vert`.

The demo detects the file timestamp and reloads the program on the next frame.

## Sample output

```text
sdf_core: PASS
lesson_catalog: PASS
lessons: 3
default_checksum: 5669664343924647534
reference_image: PASS
checksum: 5669664343924647534
```

The checksum identifies the 32 by 18 default capture.

The fixture test fails when the render exceeds its cross-platform tolerance.

## Tests

`sketchbook_sdf_tests` checks field values, dimensions, checksums, and parameter sensitivity.

`sketchbook_lesson_tests` checks lesson metadata, lookup, and wrapped navigation.

`sketchbook_reference_tests` compares the default render with `tests/fixtures/sdf_scene_reference.ppm`.

Run all registered tests with `ctest --test-dir build-vs -C Release --output-on-failure`.

CI builds and tests the project on Windows and Ubuntu.

Current status: three deterministic tests pass locally after a Release build.

## Limitations

- The interactive host currently uses Win32 and WGL.
- The reference image checks the CPU renderer, not a GPU framebuffer.
- The demo uses a compatibility OpenGL context for wide driver support.
- Lessons share the demo uniform set, but each lesson uses only the controls it needs.
- Shader reload watches file timestamps, so some editors need a saved file before reload.
- The reference capture uses a small image for fast CI feedback.

## Roadmap

- [x] 0.2: Add three named GLSL lessons with keyboard navigation and deterministic catalog tests.
- [ ] 0.3: Add framebuffer capture and GPU image comparison.
- [ ] 0.4: Add camera orbit controls and signed-distance 2D sketches.
- [ ] 0.5: Add a cross-platform window layer with an optional tested backend.

## License

MIT. See [LICENSE](LICENSE).


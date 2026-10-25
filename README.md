# Shader Sketchbook

Shader Sketchbook is a small C++ and OpenGL playground for signed-distance field lessons.

Each lesson presents one technique through live GLSL. The CPU renderer supplies deterministic reference evidence.

## Value

- Study signed-distance fields through focused lessons.
- Reload GLSL while the demo runs.
- Adjust scene parameters with keyboard controls.
- Capture the active OpenGL framebuffer.
- Compare PPM images with an explicit tolerance.
- Run lesson loading and controls without a display.

## Release 0.5: shader diagnostics

The shared lesson session owns controls, lesson selection, pause state, and backend requests.

The optional Linux X11/GLX host runs the same lessons as the Windows WGL host.

The session test validates lesson loading without a display.

Shader reload failures report the stage, source file, and normalized compiler message.

The last valid program remains active after a failed reload.

## GPU evidence

Press `C` to save the active framebuffer under `captures/<lesson-id>.ppm`.

The Windows smoke test creates a real WGL context and renders `composite_scene` at 64 x 36 pixels.

The test compares that framebuffer with a committed GPU reference image.

The tolerance is `0.08` normalized RGB mean absolute error.

CPU and GPU references remain separate evidence paths.

## Architecture

```text
Interactive Windows host
    -> Win32 window
    -> WGL context
    -> OpenGL renderer
    -> GLSL lesson
    -> PPM framebuffer capture

Linux X11/GLX host
    -> shared lesson session
    -> OpenGL renderer
    -> GLSL lesson

Portable core
    -> lesson catalog
    -> lesson session
    -> signed-distance scene
    -> CPU reference renderer
    -> PPM image comparison
    -> deterministic tests
```

| Area | Location | Role |
| --- | --- | --- |
| Scene model | `include/shader_sketchbook/sdf_scene.hpp` | Public scene settings and SDF operations |
| Image model | `include/shader_sketchbook/image.hpp` | PPM data and normalized error measurement |
| Lesson catalog | `include/shader_sketchbook/lesson_catalog.hpp` | Lesson metadata and lookup |
| Lesson session | `include/shader_sketchbook/lesson_session.hpp` | Shared controls and asset validation |
| CPU evidence | `src/core/sdf_scene.cpp` | Deterministic reference renderer |
| Image tool | `src/tools/compare_images.cpp` | Command-line PPM comparison |
| Diagnostic model | `include/shader_sketchbook/shader_diagnostic.hpp` | Stable stage and file error data |
| Windows host | `src/demo/opengl_renderer.cpp` | WGL context, rendering, and capture |
| Shader reload | `src/demo/shader_program.cpp` | Compile, reload, diagnostics, and valid-program preservation |
| GPU evidence | `tests/test_gpu_capture.cpp` | Hidden-context smoke test and reference comparison |
| Portable host | `src/demo/x11_main.cpp` | Optional Linux X11/GLX window host |
| References | `tests/fixtures/` | Separate CPU and GPU PPM fixtures |

## Setup

Use CMake 3.20 or newer.

Use a desktop OpenGL driver on Windows.

Run these commands from the repository root.

Use a new build directory after moving the repository.

```powershell
cmake -S . -B build-vs -G "Visual Studio 17 2022" -A x64
cmake --build build-vs --config Release --parallel 1
ctest --test-dir build-vs -C Release --output-on-failure
.\build-vs\Release\shader_sketchbook.exe
```

The project has no third-party runtime dependency.

Run the portable checks on Linux or macOS with these commands.

```sh
cmake -S . -B build-linux -DCMAKE_BUILD_TYPE=Release
cmake --build build-linux --parallel
ctest --test-dir build-linux --output-on-failure
```

The non-Windows executable prints a platform notice.

Build the optional Linux host on Ubuntu.

```sh
sudo apt-get install libgl1-mesa-dev libglx-dev libx11-dev
cmake -S . -B build-linux -DCMAKE_BUILD_TYPE=Release
cmake --build build-linux --parallel
./build-linux/shader_sketchbook_x11
```

The host needs an active X display.

## Controls

| Key | Action |
| --- | --- |
| `Left` / `Right` | Select a lesson |
| `,` / `.` | Select a lesson |
| `A` / `D` | Change shape blend |
| `S` / `W` | Change sphere radius |
| `E` / `Q` | Change repeat spacing |
| `Z` / `X` | Change light energy |
| `R` | Force shader reload |
| `C` | Capture the framebuffer |
| `Space` | Pause or resume animation |
| `Esc` | Close the demo |

Save an edited fragment shader before reloading it.

The renderer keeps the last valid program when compilation fails.

To compare two captures, use the same viewport size for both images.

```powershell
.\build-vs\Release\sketchbook_image_compare.exe captures\first.ppm captures\second.ppm 0.08
```

## Sample output

```text
lesson_session: PASS
lesson_count: 5
gpu_capture: PASS
dimensions: 64x36
mean_absolute_error: 0
tolerance: 0.08
image_core: PASS
single_channel_error: 0.166667
shader_diagnostics: PASS
shader_reload: PASS
```

The GPU smoke test reads pixels from the OpenGL framebuffer.

The image unit test checks exact equality, channel normalization, and dimension mismatch handling.

## Tests

CTest registers nine tests on Windows and eight tests on Linux.

| Test | Evidence |
| --- | --- |
| `sdf_core` | SDF values and parameter sensitivity |
| `reference_image` | CPU render against the CPU fixture |
| `lesson_catalog` | Lesson metadata and shader paths |
| `lesson_session` | Headless lesson loading and shared controls |
| `image_core` | Image error behavior |
| `shader_diagnostics` | Normalized stage and file diagnostic data |
| `image_compare` | Exact command-line comparison |
| `gpu_capture` | WGL framebuffer against the GPU fixture |
| `shader_reload` | Last valid program preservation with fake OpenGL calls |

Run all registered tests with this command.

```powershell
ctest --test-dir build-vs -C Release --output-on-failure
```

The GPU test skips with code 77 when a hidden WGL context is unavailable.

CI builds the Windows demo and the optional Ubuntu X11 host.

CI uploads the GPU capture and reference image from the Windows job.

Local status: nine CTest tests pass with Visual Studio 2019 and CMake 4.3.1.

## Limitations

- The Windows interactive host supports Win32 and WGL.
- The Linux host requires X11, GLX, and an active display.
- The Linux host is optional when those development files are unavailable.
- The smoke test uses a fixed 64 x 36 viewport.
- The demo opens at 1280 x 720 pixels.
- GPU pixels can vary between drivers.
- The GPU comparison uses a tolerance for driver variation.
- The CPU fixture does not prove GPU output.
- Shader reload requires a saved file.
- Compiler diagnostics collapse whitespace into one line.
- Headless CI does not run an X11 window.

## Roadmap

See [ROADMAP.md](ROADMAP.md) for milestone status, acceptance criteria, and verification commands.

## License

This project is released under the [MIT License](LICENSE).

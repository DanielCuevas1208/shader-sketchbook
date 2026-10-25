# Roadmap

This roadmap tracks bounded releases for the shader lesson workflow.

| ID | Status | User-visible outcome |
| --- | --- | --- |
| `scene-core` | Complete | Explore one configurable signed-distance scene. |
| `lesson-browser` | Complete | Browse focused GLSL lessons with live reload. |
| `gpu-capture` | Complete | Capture and review real OpenGL framebuffer output. |
| `portable-session` | Complete | Load lessons and apply controls without a display. |
| `portable-host` | Blocked | Run lessons through an optional cross-platform window backend. |
| `camera-orbit` | Planned | Orbit the camera and study a signed-distance 2D sketch. |
| `shader-diagnostics` | Complete | Understand shader reload failures from a concise diagnostic. |
| `parameter-presets` | Planned | Save and restore lesson controls between sessions. |
| `capture-review` | Planned | Explain image differences with deterministic per-channel metrics. |

## `scene-core`

Status: complete.

Deliverable: Add scene settings, SDF queries, a CPU renderer, and a committed CPU reference.

Acceptance criteria:

- Scene settings change the signed-distance field.
- The CPU renderer produces a stable image checksum.
- The reference test detects image changes.

Verification:

```text
cmake --build build-vs --config Release --parallel 1
ctest --test-dir build-vs -C Release -R "sdf_core|reference_image" --output-on-failure
```

## `lesson-browser`

Status: complete.

Deliverable: Add named GLSL lessons, keyboard selection, live reload, and parameter controls.

Acceptance criteria:

- The catalog exposes unique lesson identifiers.
- Each catalog entry resolves to an existing shader.
- The Windows demo selects lessons and preserves the last valid shader.

Verification:

```text
ctest --test-dir build-vs -C Release -R lesson_catalog --output-on-failure
```

## `gpu-capture`

Status: complete.

Deliverable: Add framebuffer capture, a PPM comparison tool, a hidden WGL smoke test, and a GPU reference.

Acceptance criteria:

- The demo saves the current framebuffer after `C` input.
- The smoke test creates a real WGL context when Windows supports it.
- The smoke test compares a 64 x 36 framebuffer with an explicit `0.08` tolerance.
- CPU and GPU references remain separately labeled.

Verification:

```text
cmake --build build-vs --config Release --parallel 1
ctest --test-dir build-vs -C Release -R "image_core|image_compare|gpu_capture" --output-on-failure
```

Evidence:

- GPU capture: `tests/fixtures/gpu_composite_reference.ppm`
- CPU reference: `tests/fixtures/sdf_scene_reference.ppm`
- Smoke test: `tests/test_gpu_capture.cpp`

Verification result: seven CTest cases pass in the local Windows `build-ci` tree.

## `portable-session`

Status: complete.

Deliverable: Add a backend-independent lesson session for asset checks, controls, and backend requests.

Acceptance criteria:

- All catalog shaders load without a display.
- Lesson selection wraps at both catalog boundaries.
- Parameter, pause, reload, and capture commands update deterministic session state.
- Reload and capture requests are consumed once.

Verification:

```text
cmake --build build-ci --config Release --parallel 1
ctest --test-dir build-ci -C Release -R lesson_session --output-on-failure
```

## `portable-host`

Status: blocked locally.

User-visible outcome: Run the same lessons through an optional portable window backend.

Bounded deliverable: Add an optional Linux X11/GLX host that uses the shared lesson session.

Acceptance criteria:

- Windows and Linux builds succeed.
- Lesson loading works without a display.
- The backend preserves shader reload and parameter controls.
- The existing Windows host keeps its behavior.

Current evidence:

- Windows build includes the shared session and existing WGL host.
- The headless session test passes without a display.
- The Linux host source is present in `src/demo/x11_main.cpp`.
- Linux compilation and display smoke testing remain CI checks.

Blocker: this workspace has no Linux runtime, X11 display, or running Docker daemon.

Verification:

```text
cmake -S . -B build-linux -DCMAKE_BUILD_TYPE=Release
cmake --build build-linux --parallel
ctest --test-dir build-linux --output-on-failure
cmake -S . -B build-vs -G "Visual Studio 17 2022" -A x64
cmake --build build-vs --config Release --parallel 1
ctest --test-dir build-vs -C Release --output-on-failure
```

```text
sudo apt-get install libgl1-mesa-dev libglx-dev libx11-dev
cmake -S . -B build-linux -DCMAKE_BUILD_TYPE=Release
cmake --build build-linux --target shader_sketchbook_x11 --parallel
ctest --test-dir build-linux --output-on-failure
```

## `camera-orbit`

Status: planned.

User-visible outcome: Orbit the camera and compare a signed-distance 2D sketch with the 3D lessons.

Bounded deliverable: Add orbit input, camera state, and one small 2D lesson with deterministic metadata tests.

Acceptance criteria:

- Mouse or keyboard input changes the camera orbit.
- Paused frames remain stable at a fixed camera state.
- The 2D lesson loads through the same catalog.
- Existing parameter controls keep their behavior.

Verification:

```text
cmake --build build-vs --config Release --parallel 1
ctest --test-dir build-vs -C Release --output-on-failure
```

## `shader-diagnostics`

Status: complete.

User-visible outcome: Identify the shader file and compile stage after a reload failure.

Bounded deliverable: Add structured shader error text and one deterministic parser test.

Acceptance criteria:

- A failed vertex or fragment compile names its stage and file.
- The last valid program remains active after a failed reload.
- The diagnostic test runs without a display.

Verification:

```text
cmake --build build-vs16 --config Release --parallel 1
ctest --test-dir build-vs16 -C Release -R "shader_diagnostics|shader_reload" --output-on-failure
```

Evidence:

- Diagnostic model: `include/shader_sketchbook/shader_diagnostic.hpp`
- Parser test: `tests/test_shader_diagnostics.cpp`
- Reload regression test: `tests/test_shader_reload.cpp`
- Verification result: nine CTest cases pass in the local Windows `build-vs16` tree.

## `parameter-presets`

Status: planned.

User-visible outcome: Save and restore lesson controls between sessions.

Bounded deliverable: Add a small text preset format and headless session tests.

Acceptance criteria:

- Valid presets restore all four scene parameters.
- Invalid values return a clear status.
- Preset loading works without a display.

Verification:

```text
cmake --build build-vs16 --config Release --parallel 1
ctest --test-dir build-vs16 -C Release -R parameter_presets --output-on-failure
```

## `capture-review`

Status: planned.

User-visible outcome: Explain image differences with deterministic per-channel metrics.

Bounded deliverable: Extend the comparison tool with differing-pixel and maximum-channel metrics.

Acceptance criteria:

- Identical images report zero differences.
- Dimension mismatches return a nonzero status.
- The existing mean-error threshold remains supported.

Verification:

```text
cmake --build build-vs16 --config Release --parallel 1
ctest --test-dir build-vs16 -C Release -R capture_review --output-on-failure
```

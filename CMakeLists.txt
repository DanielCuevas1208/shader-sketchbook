cmake_minimum_required(VERSION 3.20)

project(shader_sketchbook VERSION 0.5.0 LANGUAGES CXX)

option(SHADER_SKETCHBOOK_BUILD_TESTS "Build deterministic core, reference, and GPU smoke tests" ON)
option(SHADER_SKETCHBOOK_BUILD_PORTABLE_HOST "Build the optional Linux X11/GLX lesson host" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(sketchbook_core STATIC
    src/core/image.cpp
    src/core/lesson_catalog.cpp
    src/core/lesson_session.cpp
    src/core/shader_diagnostic.cpp
    src/core/sdf_scene.cpp
)

target_include_directories(sketchbook_core PUBLIC include)

if(MSVC)
    target_compile_options(sketchbook_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(sketchbook_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

add_executable(sketchbook_image_compare src/tools/compare_images.cpp)
target_link_libraries(sketchbook_image_compare PRIVATE sketchbook_core)

if(SHADER_SKETCHBOOK_BUILD_TESTS)
    enable_testing()

    add_executable(sketchbook_sdf_tests tests/test_sdf_scene.cpp)
    target_link_libraries(sketchbook_sdf_tests PRIVATE sketchbook_core)

    add_executable(sketchbook_reference_tests tests/test_image_reference.cpp)
    target_link_libraries(sketchbook_reference_tests PRIVATE sketchbook_core)
    target_compile_definitions(sketchbook_reference_tests PRIVATE
        SKETCHBOOK_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures")

    add_executable(sketchbook_lesson_tests tests/test_lesson_catalog.cpp)
    target_link_libraries(sketchbook_lesson_tests PRIVATE sketchbook_core)
    target_compile_definitions(sketchbook_lesson_tests PRIVATE
        SKETCHBOOK_LESSON_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders/lessons")
    add_executable(sketchbook_lesson_session_tests tests/test_lesson_session.cpp)
    target_link_libraries(sketchbook_lesson_session_tests PRIVATE sketchbook_core)
    target_compile_definitions(sketchbook_lesson_session_tests PRIVATE
        SKETCHBOOK_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders")
    add_executable(sketchbook_image_tests tests/test_image.cpp)
    target_link_libraries(sketchbook_image_tests PRIVATE sketchbook_core)
    add_executable(sketchbook_shader_diagnostic_tests tests/test_shader_diagnostics.cpp)
    target_link_libraries(sketchbook_shader_diagnostic_tests PRIVATE sketchbook_core)

    add_test(NAME sdf_core COMMAND sketchbook_sdf_tests)
    add_test(NAME reference_image COMMAND sketchbook_reference_tests)
    add_test(NAME lesson_catalog COMMAND sketchbook_lesson_tests)
    add_test(NAME lesson_session COMMAND sketchbook_lesson_session_tests)
    add_test(NAME image_core COMMAND sketchbook_image_tests)
    add_test(NAME shader_diagnostics COMMAND sketchbook_shader_diagnostic_tests)
    add_test(NAME image_compare COMMAND sketchbook_image_compare
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/sdf_scene_reference.ppm"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/sdf_scene_reference.ppm"
        "0.0")
endif()

if(WIN32)
    add_executable(shader_sketchbook WIN32
        src/demo/main.cpp
        src/demo/native_window.cpp
        src/demo/opengl_renderer.cpp
        src/demo/shader_program.cpp
    )
    target_include_directories(shader_sketchbook PRIVATE src/demo include)
    target_compile_definitions(shader_sketchbook PRIVATE
        SKETCHBOOK_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders")
    target_link_libraries(shader_sketchbook PRIVATE opengl32 user32 gdi32 sketchbook_core)
    if(MSVC)
        target_compile_options(shader_sketchbook PRIVATE /W4 /permissive-)
    else()
        target_compile_options(shader_sketchbook PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    if(SHADER_SKETCHBOOK_BUILD_TESTS)
        add_executable(sketchbook_gpu_tests
            tests/test_gpu_capture.cpp
            src/demo/native_window.cpp
            src/demo/opengl_renderer.cpp
            src/demo/shader_program.cpp
        )
        target_include_directories(sketchbook_gpu_tests PRIVATE src/demo include)
        target_compile_definitions(sketchbook_gpu_tests PRIVATE
            SKETCHBOOK_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
            SKETCHBOOK_GPU_FIXTURE="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/gpu_composite_reference.ppm")
        target_link_libraries(sketchbook_gpu_tests PRIVATE opengl32 user32 gdi32 sketchbook_core)
        if(MSVC)
            target_compile_options(sketchbook_gpu_tests PRIVATE /W4 /permissive-)
        else()
            target_compile_options(sketchbook_gpu_tests PRIVATE -Wall -Wextra -Wpedantic)
        endif()
        add_test(NAME gpu_capture COMMAND sketchbook_gpu_tests)
        add_executable(sketchbook_shader_reload_tests
            tests/test_shader_reload.cpp
            src/demo/shader_program.cpp
        )
        target_include_directories(sketchbook_shader_reload_tests PRIVATE src/demo include)
        target_link_libraries(sketchbook_shader_reload_tests PRIVATE opengl32 sketchbook_core)
        if(MSVC)
            target_compile_options(sketchbook_shader_reload_tests PRIVATE /W4 /permissive-)
        else()
            target_compile_options(sketchbook_shader_reload_tests PRIVATE -Wall -Wextra -Wpedantic)
        endif()
        add_test(NAME shader_reload COMMAND sketchbook_shader_reload_tests)
        set_tests_properties(gpu_capture PROPERTIES
            SKIP_RETURN_CODE 77
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    endif()
else()
    add_executable(shader_sketchbook src/demo/unsupported_platform.cpp)
    target_compile_features(shader_sketchbook PRIVATE cxx_std_17)
endif()

if(UNIX AND NOT APPLE AND SHADER_SKETCHBOOK_BUILD_PORTABLE_HOST)
    find_package(OpenGL QUIET)
    find_package(X11 QUIET)
    if(OpenGL_FOUND AND X11_FOUND)
        add_executable(shader_sketchbook_x11
            src/demo/x11_main.cpp
            src/demo/shader_program.cpp
        )
        target_include_directories(shader_sketchbook_x11 PRIVATE
            src/demo
            include
            ${X11_INCLUDE_DIR})
        target_compile_definitions(shader_sketchbook_x11 PRIVATE
            SKETCHBOOK_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders")
        target_link_libraries(shader_sketchbook_x11 PRIVATE
            sketchbook_core
            ${X11_LIBRARIES}
            OpenGL::GL)
        target_compile_options(shader_sketchbook_x11 PRIVATE -Wall -Wextra -Wpedantic)
    else()
        message(STATUS "Portable X11 host disabled: OpenGL or X11 development files are unavailable")
    endif()
endif()

set_target_properties(sketchbook_core PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES)

cmake_minimum_required(VERSION 3.20)

project(shader_sketchbook VERSION 0.2.0 LANGUAGES CXX)

option(SHADER_SKETCHBOOK_BUILD_TESTS "Build deterministic core and reference tests" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(sketchbook_core STATIC
    src/core/image.cpp
    src/core/lesson_catalog.cpp
    src/core/sdf_scene.cpp
)

target_include_directories(sketchbook_core PUBLIC include)

if(MSVC)
    target_compile_options(sketchbook_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(sketchbook_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

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

    add_test(NAME sdf_core COMMAND sketchbook_sdf_tests)
    add_test(NAME reference_image COMMAND sketchbook_reference_tests)
    add_test(NAME lesson_catalog COMMAND sketchbook_lesson_tests)
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
else()
    add_executable(shader_sketchbook src/demo/unsupported_platform.cpp)
    target_compile_features(shader_sketchbook PRIVATE cxx_std_17)
endif()

set_target_properties(sketchbook_core PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES)

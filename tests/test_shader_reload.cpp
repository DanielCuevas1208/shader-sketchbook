#include "shader_program.hpp"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {

struct FakeGlState {
    bool fail_fragment = false;
    GLuint next_shader = 1;
    GLuint next_program = 100;
    GLuint fragment_shader = 0;
    int deleted_programs = 0;
};

FakeGlState* state = nullptr;

void require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

GLuint APIENTRY fake_create_shader(GLenum type) {
    const GLuint shader = state->next_shader++;
    if (type == GL_FRAGMENT_SHADER) {
        state->fragment_shader = shader;
    }
    return shader;
}

void APIENTRY fake_shader_source(GLuint, GLsizei, const char* const*, const GLint*) {}
void APIENTRY fake_compile_shader(GLuint) {}

void APIENTRY fake_get_shader_iv(GLuint shader, GLenum name, GLint* value) {
    if (name == GL_COMPILE_STATUS) {
        *value = state->fail_fragment && shader == state->fragment_shader ? GL_FALSE : GL_TRUE;
    } else {
        *value = 16;
    }
}

void APIENTRY fake_get_shader_info_log(GLuint, GLsizei length, GLsizei* written, char* log) {
    const std::string message = "syntax error";
    const std::size_t count = std::min<std::size_t>(message.size(), static_cast<std::size_t>(length));
    std::copy_n(message.data(), count, log);
    *written = static_cast<GLsizei>(count);
}

void APIENTRY fake_delete_shader(GLuint) {}

GLuint APIENTRY fake_create_program() {
    return state->next_program++;
}

void APIENTRY fake_attach_shader(GLuint, GLuint) {}
void APIENTRY fake_link_program(GLuint) {}

void APIENTRY fake_get_program_iv(GLuint, GLenum name, GLint* value) {
    if (name == GL_LINK_STATUS) {
        *value = GL_TRUE;
    } else {
        *value = 0;
    }
}

void APIENTRY fake_get_program_info_log(GLuint, GLsizei, GLsizei* written, char*) {
    *written = 0;
}

void APIENTRY fake_delete_program(GLuint) {
    ++state->deleted_programs;
}

void APIENTRY fake_use_program(GLuint) {}
GLint APIENTRY fake_get_uniform_location(GLuint, const char*) { return -1; }
void APIENTRY fake_uniform_1f(GLint, GLfloat) {}
void APIENTRY fake_uniform_2f(GLint, GLfloat, GLfloat) {}

shader_sketchbook::demo::GlApi make_api() {
    shader_sketchbook::demo::GlApi api{};
    api.create_shader = &fake_create_shader;
    api.shader_source = &fake_shader_source;
    api.compile_shader = &fake_compile_shader;
    api.get_shader_iv = &fake_get_shader_iv;
    api.get_shader_info_log = &fake_get_shader_info_log;
    api.delete_shader = &fake_delete_shader;
    api.create_program = &fake_create_program;
    api.attach_shader = &fake_attach_shader;
    api.link_program = &fake_link_program;
    api.get_program_iv = &fake_get_program_iv;
    api.get_program_info_log = &fake_get_program_info_log;
    api.delete_program = &fake_delete_program;
    api.use_program = &fake_use_program;
    api.get_uniform_location = &fake_get_uniform_location;
    api.uniform_1f = &fake_uniform_1f;
    api.uniform_2f = &fake_uniform_2f;
    return api;
}

}  // namespace

int main() {
    using namespace shader_sketchbook::demo;

    const std::filesystem::path root =
        std::filesystem::temp_directory_path() / "shader_sketchbook_shader_reload_test";
    std::filesystem::create_directories(root);
    const std::filesystem::path vertex = root / "fullscreen.vert";
    const std::filesystem::path fragment = root / "lesson.frag";
    std::ofstream(vertex) << "void main() {}";
    std::ofstream(fragment) << "void main() {}";

    FakeGlState fake_state{};
    state = &fake_state;
    GlApi api = make_api();
    ShaderProgram program(api);
    require(program.load(vertex, fragment), "a valid shader pair loads");
    require(program.valid(), "a loaded program is valid");

    fake_state.fail_fragment = true;
    require(!program.force_reload(), "a failed reload reports failure");
    require(program.valid(), "a failed reload keeps the last valid program");
    require(fake_state.deleted_programs == 0, "a failed reload does not delete the active program");
    require(program.last_error().find("stage=fragment") != std::string::npos,
            "a fragment compile failure names its stage");
    require(program.last_error().find(fragment.string()) != std::string::npos,
            "a fragment compile failure names its file");

    program.destroy();
    require(fake_state.deleted_programs == 1, "destroy releases the active program once");
    std::filesystem::remove_all(root);

    std::cout << "shader_reload: PASS\n";
    return EXIT_SUCCESS;
}

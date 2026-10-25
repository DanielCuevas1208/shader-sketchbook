#include "shader_program.hpp"

#ifndef _WIN32
#include <GL/glx.h>
#endif

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

namespace shader_sketchbook::demo {
namespace {

shader_sketchbook::ShaderStage stage_for_type(GLenum type) {
    return type == GL_VERTEX_SHADER ? shader_sketchbook::ShaderStage::Vertex
                                    : shader_sketchbook::ShaderStage::Fragment;
}

template <typename Function>
Function load_function(const char* name) {
#ifdef _WIN32
    PROC address = wglGetProcAddress(name);
    if (address == nullptr) {
        const HMODULE opengl = GetModuleHandleA("opengl32.dll");
        address = GetProcAddress(opengl, name);
    }
    return reinterpret_cast<Function>(address);
#else
    const auto address = glXGetProcAddressARB(reinterpret_cast<const GLubyte*>(name));
    return reinterpret_cast<Function>(address);
#endif
}

bool file_timestamp(const std::filesystem::path& path, std::filesystem::file_time_type& result) {
    std::error_code error;
    result = std::filesystem::last_write_time(path, error);
    return !error;
}

}  // namespace

bool GlApi::load() {
    create_shader = load_function<CreateShader>("glCreateShader");
    shader_source = load_function<ShaderSource>("glShaderSource");
    compile_shader = load_function<CompileShader>("glCompileShader");
    get_shader_iv = load_function<GetShaderiv>("glGetShaderiv");
    get_shader_info_log = load_function<GetShaderInfoLog>("glGetShaderInfoLog");
    delete_shader = load_function<DeleteShader>("glDeleteShader");
    create_program = load_function<CreateProgram>("glCreateProgram");
    attach_shader = load_function<AttachShader>("glAttachShader");
    link_program = load_function<LinkProgram>("glLinkProgram");
    get_program_iv = load_function<GetProgramiv>("glGetProgramiv");
    get_program_info_log = load_function<GetProgramInfoLog>("glGetProgramInfoLog");
    delete_program = load_function<DeleteProgram>("glDeleteProgram");
    use_program = load_function<UseProgram>("glUseProgram");
    get_uniform_location = load_function<GetUniformLocation>("glGetUniformLocation");
    uniform_1f = load_function<Uniform1f>("glUniform1f");
    uniform_2f = load_function<Uniform2f>("glUniform2f");

    return create_shader != nullptr && shader_source != nullptr && compile_shader != nullptr &&
           get_shader_iv != nullptr && get_shader_info_log != nullptr && delete_shader != nullptr &&
           create_program != nullptr && attach_shader != nullptr && link_program != nullptr &&
           get_program_iv != nullptr && get_program_info_log != nullptr && delete_program != nullptr &&
           use_program != nullptr && get_uniform_location != nullptr && uniform_1f != nullptr && uniform_2f != nullptr;
}

GLuint ShaderProgram::compile(GLenum type, const std::filesystem::path& path, std::string& error) const {
    const shader_sketchbook::ShaderStage stage = stage_for_type(type);
    std::ifstream file(path);
    if (!file) {
        error = shader_sketchbook::format_shader_diagnostic(
            shader_sketchbook::parse_shader_diagnostic(stage, path, "could not open shader"));
        return 0;
    }
    const std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (source.empty()) {
        error = shader_sketchbook::format_shader_diagnostic(
            shader_sketchbook::parse_shader_diagnostic(stage, path, "shader is empty"));
        return 0;
    }

    const GLuint shader = api_.create_shader(type);
    const char* source_pointer = source.c_str();
    api_.shader_source(shader, 1, &source_pointer, nullptr);
    api_.compile_shader(shader);

    GLint compiled = GL_FALSE;
    api_.get_shader_iv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
        GLint log_length = 0;
        api_.get_shader_iv(shader, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> log(static_cast<std::size_t>(std::max(log_length, 1)));
        GLsizei written = 0;
        api_.get_shader_info_log(shader, static_cast<GLsizei>(log.size()), &written, log.data());
        error = shader_sketchbook::format_shader_diagnostic(shader_sketchbook::parse_shader_diagnostic(
            stage, path, std::string_view(log.data(), static_cast<std::size_t>(std::max(written, 0)))));
        api_.delete_shader(shader);
        return 0;
    }
    return shader;
}

std::string ShaderProgram::program_log(GLuint program) const {
    GLint log_length = 0;
    api_.get_program_iv(program, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length <= 1) {
        return {};
    }
    std::vector<char> log(static_cast<std::size_t>(log_length));
    GLsizei written = 0;
    api_.get_program_info_log(program, static_cast<GLsizei>(log.size()), &written, log.data());
    return std::string(log.data(), static_cast<std::size_t>(std::max(written, 0)));
}

bool ShaderProgram::load(const std::filesystem::path& vertex_path, const std::filesystem::path& fragment_path) {
    std::string error;
    const GLuint vertex = compile(GL_VERTEX_SHADER, vertex_path, error);
    if (vertex == 0) {
        last_error_ = error;
        std::cerr << "shader: " << last_error_ << '\n';
        return false;
    }
    const GLuint fragment = compile(GL_FRAGMENT_SHADER, fragment_path, error);
    if (fragment == 0) {
        api_.delete_shader(vertex);
        last_error_ = error;
        std::cerr << "shader: " << last_error_ << '\n';
        return false;
    }

    const GLuint candidate = api_.create_program();
    api_.attach_shader(candidate, vertex);
    api_.attach_shader(candidate, fragment);
    api_.link_program(candidate);
    api_.delete_shader(vertex);
    api_.delete_shader(fragment);

    GLint linked = GL_FALSE;
    api_.get_program_iv(candidate, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE) {
        last_error_ = shader_sketchbook::format_shader_diagnostic(
            shader_sketchbook::parse_shader_diagnostic(shader_sketchbook::ShaderStage::Program,
                                                        fragment_path,
                                                        program_log(candidate)));
        api_.delete_program(candidate);
        std::cerr << "shader: " << last_error_ << '\n';
        return false;
    }

    if (program_ != 0) {
        api_.delete_program(program_);
    }
    program_ = candidate;
    vertex_path_ = vertex_path;
    fragment_path_ = fragment_path;
    file_timestamp(vertex_path_, vertex_write_time_);
    file_timestamp(fragment_path_, fragment_write_time_);
    last_error_.clear();
    std::cout << "shader: loaded " << fragment_path_.filename().string() << '\n';
    return true;
}

bool ShaderProgram::reload_if_changed() {
    if (program_ == 0) {
        return false;
    }
    std::filesystem::file_time_type vertex_time{};
    std::filesystem::file_time_type fragment_time{};
    if (!file_timestamp(vertex_path_, vertex_time) || !file_timestamp(fragment_path_, fragment_time)) {
        return false;
    }
    if (vertex_time != vertex_write_time_ || fragment_time != fragment_write_time_) {
        return load(vertex_path_, fragment_path_);
    }
    return false;
}

bool ShaderProgram::force_reload() {
    if (vertex_path_.empty() || fragment_path_.empty()) {
        return false;
    }
    return load(vertex_path_, fragment_path_);
}

void ShaderProgram::destroy() noexcept {
    if (program_ != 0) {
        api_.delete_program(program_);
        program_ = 0;
    }
}

void ShaderProgram::bind() const noexcept {
    api_.use_program(program_);
}

GLint ShaderProgram::uniform(const char* name) const noexcept {
    return api_.get_uniform_location(program_, name);
}

}  // namespace shader_sketchbook::demo





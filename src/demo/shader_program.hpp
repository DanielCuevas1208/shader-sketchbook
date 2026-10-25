#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#else
#include <GL/gl.h>
#ifndef APIENTRY
#define APIENTRY
#endif
#endif

#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#endif

#include "shader_sketchbook/shader_diagnostic.hpp"

#include <filesystem>
#include <string>

namespace shader_sketchbook::demo {

struct GlApi {
    using CreateShader = GLuint(APIENTRY*)(GLenum);
    using ShaderSource = void(APIENTRY*)(GLuint, GLsizei, const char* const*, const GLint*);
    using CompileShader = void(APIENTRY*)(GLuint);
    using GetShaderiv = void(APIENTRY*)(GLuint, GLenum, GLint*);
    using GetShaderInfoLog = void(APIENTRY*)(GLuint, GLsizei, GLsizei*, char*);
    using DeleteShader = void(APIENTRY*)(GLuint);
    using CreateProgram = GLuint(APIENTRY*)();
    using AttachShader = void(APIENTRY*)(GLuint, GLuint);
    using LinkProgram = void(APIENTRY*)(GLuint);
    using GetProgramiv = void(APIENTRY*)(GLuint, GLenum, GLint*);
    using GetProgramInfoLog = void(APIENTRY*)(GLuint, GLsizei, GLsizei*, char*);
    using DeleteProgram = void(APIENTRY*)(GLuint);
    using UseProgram = void(APIENTRY*)(GLuint);
    using GetUniformLocation = GLint(APIENTRY*)(GLuint, const char*);
    using Uniform1f = void(APIENTRY*)(GLint, GLfloat);
    using Uniform2f = void(APIENTRY*)(GLint, GLfloat, GLfloat);

    CreateShader create_shader = nullptr;
    ShaderSource shader_source = nullptr;
    CompileShader compile_shader = nullptr;
    GetShaderiv get_shader_iv = nullptr;
    GetShaderInfoLog get_shader_info_log = nullptr;
    DeleteShader delete_shader = nullptr;
    CreateProgram create_program = nullptr;
    AttachShader attach_shader = nullptr;
    LinkProgram link_program = nullptr;
    GetProgramiv get_program_iv = nullptr;
    GetProgramInfoLog get_program_info_log = nullptr;
    DeleteProgram delete_program = nullptr;
    UseProgram use_program = nullptr;
    GetUniformLocation get_uniform_location = nullptr;
    Uniform1f uniform_1f = nullptr;
    Uniform2f uniform_2f = nullptr;

    bool load();
};

class ShaderProgram {
public:
    explicit ShaderProgram(GlApi& api) : api_(api) {}
    ~ShaderProgram() = default;

    bool load(const std::filesystem::path& vertex_path, const std::filesystem::path& fragment_path);
    bool reload_if_changed();
    bool force_reload();
    void destroy() noexcept;
    void bind() const noexcept;
    GLint uniform(const char* name) const noexcept;
    bool valid() const noexcept { return program_ != 0; }
    const std::string& last_error() const noexcept { return last_error_; }

private:
    GLuint compile(GLenum type, const std::filesystem::path& path, std::string& error) const;
    std::string program_log(GLuint program) const;

    GlApi& api_;
    GLuint program_ = 0;
    std::filesystem::path vertex_path_;
    std::filesystem::path fragment_path_;
    std::filesystem::file_time_type vertex_write_time_{};
    std::filesystem::file_time_type fragment_write_time_{};
    std::string last_error_;
};

}  // namespace shader_sketchbook::demo




#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace shader_sketchbook {

enum class ShaderStage {
    Vertex,
    Fragment,
    Program,
};

struct ShaderDiagnostic {
    ShaderStage stage;
    std::filesystem::path source;
    std::string message;
};

const char* shader_stage_name(ShaderStage stage) noexcept;

ShaderDiagnostic parse_shader_diagnostic(ShaderStage stage,
                                         const std::filesystem::path& source,
                                         std::string_view compiler_log);

std::string format_shader_diagnostic(const ShaderDiagnostic& diagnostic);

}  // namespace shader_sketchbook

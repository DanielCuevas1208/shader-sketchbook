#include "shader_sketchbook/shader_diagnostic.hpp"

#include <cctype>

namespace shader_sketchbook {
namespace {

std::string normalize_message(std::string_view message) {
    std::string normalized;
    normalized.reserve(message.size());
    bool pending_space = false;

    for (const char character : message) {
        if (std::isspace(static_cast<unsigned char>(character)) != 0) {
            pending_space = !normalized.empty();
            continue;
        }
        if (pending_space) {
            normalized.push_back(' ');
            pending_space = false;
        }
        normalized.push_back(character);
    }
    return normalized;
}

}  // namespace

const char* shader_stage_name(ShaderStage stage) noexcept {
    switch (stage) {
    case ShaderStage::Vertex: return "vertex";
    case ShaderStage::Fragment: return "fragment";
    case ShaderStage::Program: return "program";
    }
    return "unknown";
}

ShaderDiagnostic parse_shader_diagnostic(ShaderStage stage,
                                         const std::filesystem::path& source,
                                         std::string_view compiler_log) {
    ShaderDiagnostic diagnostic{stage, source, normalize_message(compiler_log)};
    if (diagnostic.message.empty()) {
        diagnostic.message = "no compiler log";
    }
    return diagnostic;
}

std::string format_shader_diagnostic(const ShaderDiagnostic& diagnostic) {
    return "shader diagnostic | stage=" + std::string(shader_stage_name(diagnostic.stage)) +
           " | file=" + diagnostic.source.string() + " | message=" + diagnostic.message;
}

}  // namespace shader_sketchbook

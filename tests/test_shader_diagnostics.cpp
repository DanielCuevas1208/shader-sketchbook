#include "shader_sketchbook/shader_diagnostic.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

}  // namespace

int main() {
    using namespace shader_sketchbook;

    const std::filesystem::path source = "lessons/fullscreen.vert";
    const ShaderDiagnostic diagnostic =
        parse_shader_diagnostic(ShaderStage::Vertex, source, "  ERROR: 0:2: missing semicolon\r\n");
    require(diagnostic.stage == ShaderStage::Vertex, "the parser preserves the shader stage");
    require(diagnostic.source == source, "the parser preserves the source path");
    require(diagnostic.message == "ERROR: 0:2: missing semicolon", "the parser normalizes compiler whitespace");
    require(shader_stage_name(ShaderStage::Fragment) == std::string("fragment"),
            "fragment diagnostics use a stable stage name");
    require(format_shader_diagnostic(diagnostic) ==
                "shader diagnostic | stage=vertex | file=" + source.string() +
                    " | message=ERROR: 0:2: missing semicolon",
            "formatted diagnostics include stage, file, and message");

    const ShaderDiagnostic empty = parse_shader_diagnostic(ShaderStage::Program, source, " \n\t");
    require(empty.message == "no compiler log", "empty compiler logs use a useful fallback");

    std::cout << "shader_diagnostics: PASS\n";
    return EXIT_SUCCESS;
}

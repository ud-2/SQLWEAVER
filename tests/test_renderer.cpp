#include "renderer/MarkdownRenderer.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace sqlweaver;

TEST_CASE("MarkdownRenderer formats tables and errors into valid Markdown", "[Renderer]") {
    TableManager tm;
    ProcedureManager pm;
    std::vector<ParseError> errors;
    std::vector<LintViolation> violations;

    tm.register_table(Table{
        "users",
        "hr",
        IoType::In,
        ActionType::Call,
        {SourceLocation{10, 5, 0}},
        {}
    });

    errors.push_back(ParseError{
        SourceLocation{15, 2, 0},
        "Unexpected token 'FORM'"
    });

    MarkdownRenderer renderer;
    std::string output = renderer.render(tm, pm, errors, violations);

    REQUIRE_FALSE(output.empty());
    REQUIRE(output.find("# SQLWeaver Analysis Report") != std::string::npos);
    REQUIRE(output.find("hr.users") != std::string::npos);
    REQUIRE(output.find("READ (In)") != std::string::npos);
    REQUIRE(output.find("Unexpected token 'FORM'") != std::string::npos);
}
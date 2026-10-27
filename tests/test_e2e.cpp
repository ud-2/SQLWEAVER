#include "analyzer/Analyzer.hpp"
#include "renderer/MarkdownRenderer.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace sqlweaver;

TEST_CASE("End-to-End Analysis Pipeline processes multi-block script", "[E2E]") {
    Analyzer analyzer;

    std::string full_script = 
        "-- Block 1: DDL\n"
        "CREATE TABLE employees (\n"
        "    id NUMBER,\n"
        "    name VARCHAR2(50)\n"
        ");\n"
        "/\n"
        "\n"
        "-- Block 2: DML Read & Write\n"
        "INSERT INTO employees VALUES (1, 'Alice');\n"
        "SELECT * FROM employees;\n"
        "/\n"
        "\n"
        "-- Block 3: Syntax error\n"
        "SELECT * FORM employees;\n";

    bool success = analyzer.analyze_string(full_script);
    REQUIRE(success);

    const auto& tm = analyzer.get_table_manager();
    const auto& errors = analyzer.get_errors();

    // Check extracted tables
    REQUIRE_FALSE(tm.get_tables().empty());
    
    // Check that syntax error was caught on block 3
    REQUIRE_FALSE(errors.empty());

    // Check renderer output
    MarkdownRenderer renderer;
    std::string report = analyzer.render_report(renderer);

    REQUIRE(report.find("employees") != std::string::npos);
    REQUIRE(report.find("SQLWeaver Analysis Report") != std::string::npos);
}
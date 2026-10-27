#include <catch2/catch_test_macros.hpp>
#include "preprocessor/Preprocessor.hpp"

using namespace sqlweaver;

TEST_CASE("Preprocessor normalizes Windows line endings", "[Preprocessor]") {
    Preprocessor prep;
    std::string windows_sql = "SELECT * FROM users;\r\n/\r\nSELECT * FROM orders;\r\n";

    auto blocks = prep.process(windows_sql);

    REQUIRE(blocks.size() == 2);
    REQUIRE(blocks[0].start_line == 1);
    REQUIRE(blocks[1].start_line == 3);
}

TEST_CASE("Preprocessor correctly calculates line offsets for multiple blocks", "[Preprocessor]") {
    Preprocessor prep;
    std::string sql = 
        "-- First block\n"          // Line 1
        "CREATE TABLE users (\n"    // Line 2
        "    id NUMBER\n"           // Line 3
        ");\n"                      // Line 4
        "/\n"                       // Line 5 (Delimiter)
        "\n"                        // Line 6 (Blank)
        "-- Second block\n"         // Line 7
        "SELECT * FROM users;\n";   // Line 8

    auto blocks = prep.process(sql);

    REQUIRE(blocks.size() == 2);
    
    // First block starts at line 1
    REQUIRE(blocks[0].start_line == 1);
    
    // Second block starts at line 7 (skipping the '/' and blank line 6)
    REQUIRE(blocks[1].start_line == 7);
}

TEST_CASE("Preprocessor handles trailing block without delimiter", "[Preprocessor]") {
    Preprocessor prep;
    std::string sql = "SELECT 1 FROM DUAL;";

    auto blocks = prep.process(sql);

    REQUIRE(blocks.size() == 1);
    REQUIRE(blocks[0].start_line == 1);
    REQUIRE(blocks[0].content == "SELECT 1 FROM DUAL;\n");
}
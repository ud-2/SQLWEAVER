// Include ANTLR headers first
#include "lexer/LexerStreamer.hpp"
#include "parser/PlSqlParserBase.h"
#include "PlSqlParser.h"
#include "parser/ParserWrapper.hpp"

// Include Catch2 headers second
#include <catch2/catch_test_macros.hpp>

// Undefine Catch2's CHECK macro to avoid collision with SQL 'CHECK' keyword in ANTLR
#undef CHECK

using namespace sqlweaver;

TEST_CASE("ParserWrapper successfully generates AST for valid SQL", "[Parser]") {
    LexerStreamer streamer;
    ParserWrapper parser_wrapper;

    std::string valid_sql = "SELECT * FROM users;";
    auto token_result = streamer.tokenize(valid_sql);
    auto parse_result = parser_wrapper.parse(token_result.tokens.get(), 1);

    REQUIRE(parse_result.ast != nullptr);
    REQUIRE(parse_result.errors.empty());
}

TEST_CASE("ParserWrapper captures syntax error and maps line offset correctly", "[Parser]") {
    LexerStreamer streamer;
    ParserWrapper parser_wrapper;

    // Syntax error: 'FORM' instead of 'FROM'
    std::string invalid_sql = "SELECT * FORM users;";
    
    // Simulate this block starting at line 10 in original file
    size_t block_start_line = 10;
    
    auto token_result = streamer.tokenize(invalid_sql);
    auto parse_result = parser_wrapper.parse(token_result.tokens.get(), block_start_line);

    REQUIRE_FALSE(parse_result.errors.empty());
    
    // Line number should be offset to line 10 (since error is on 1st line of block)
    REQUIRE(parse_result.errors[0].location.line == 10);
}
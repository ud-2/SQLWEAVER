#include "lexer/LexerStreamer.hpp"
#include "lexer/PlSqlLexerBase.h"
#include "PlSqlLexer.h"

#include <catch2/catch_test_macros.hpp>

#undef CHECK

using namespace sqlweaver;

TEST_CASE("LexerStreamer tokenizes lowercase keywords correctly", "[Lexer]") {
    LexerStreamer streamer;
    std::string sql = "select * from users;";

    auto result = streamer.tokenize(sql);
    auto tokens = result.tokens->getTokens();

    REQUIRE_FALSE(tokens.empty());
    REQUIRE(tokens[0]->getType() == PlSqlLexer::SELECT);
    REQUIRE(tokens[0]->getText() == "select");
}

TEST_CASE("LexerStreamer handles mixed case SQL statements", "[Lexer]") {
    LexerStreamer streamer;
    std::string sql = "SeLeCt name FrOm my_table;";

    auto result = streamer.tokenize(sql);
    auto tokens = result.tokens->getTokens();

    REQUIRE_FALSE(tokens.empty());
    REQUIRE(tokens[0]->getType() == PlSqlLexer::SELECT);
    REQUIRE(tokens[4]->getType() == PlSqlLexer::FROM);
}
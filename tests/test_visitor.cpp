#include "lexer/LexerStreamer.hpp"
#include "parser/PlSqlParserBase.h"
#include "PlSqlParser.h"
#include "parser/ParserWrapper.hpp"
#include "visitor/PlSqlVisitor.hpp"

#include <catch2/catch_test_macros.hpp>

#undef CHECK

using namespace sqlweaver;

TEST_CASE("PlSqlVisitor extracts SELECT table as IoType::In", "[Visitor]") {
    LexerStreamer streamer;
    ParserWrapper parser_wrapper;
    TableManager tm;
    ProcedureManager pm;

    std::string sql = "SELECT id, name FROM hr.users;";
    auto token_result = streamer.tokenize(sql);
    auto parse_result = parser_wrapper.parse(token_result.tokens.get(), 1);

    PlSqlVisitor visitor(tm, pm, 1);
    visitor.visit(parse_result.ast);

    const auto& tables = tm.get_tables();
    REQUIRE(tables.size() == 1);
    REQUIRE(tables[0].name == "users");
    REQUIRE(tables[0].schema == "hr");
    REQUIRE(tables[0].io_type == IoType::In);
    REQUIRE(tables[0].action == ActionType::Call);
}

TEST_CASE("PlSqlVisitor extracts INSERT table as IoType::Out", "[Visitor]") {
    LexerStreamer streamer;
    ParserWrapper parser_wrapper;
    TableManager tm;
    ProcedureManager pm;

    std::string sql = "INSERT INTO orders VALUES (100, 'PENDING');";
    auto token_result = streamer.tokenize(sql);
    auto parse_result = parser_wrapper.parse(token_result.tokens.get(), 1);

    PlSqlVisitor visitor(tm, pm, 5);
    visitor.visit(parse_result.ast);

    const auto& tables = tm.get_tables();
    REQUIRE(tables.size() == 1);
    REQUIRE(tables[0].name == "orders");
    REQUIRE(tables[0].io_type == IoType::Out);
    REQUIRE(tables[0].locations[0].line == 5);
}

TEST_CASE("PlSqlVisitor extracts CREATE TABLE definition with columns", "[Visitor]") {
    LexerStreamer streamer;
    ParserWrapper parser_wrapper;
    TableManager tm;
    ProcedureManager pm;

    std::string sql = "CREATE TABLE employees (emp_id NUMBER, salary NUMBER DEFAULT 0);";
    auto token_result = streamer.tokenize(sql);
    auto parse_result = parser_wrapper.parse(token_result.tokens.get(), 1);

    PlSqlVisitor visitor(tm, pm, 1);
    visitor.visit(parse_result.ast);

    const auto& tables = tm.get_tables();
    REQUIRE(tables.size() == 1);
    REQUIRE(tables[0].name == "employees");
    REQUIRE(tables[0].action == ActionType::Definition);
    REQUIRE(tables[0].columns.size() == 2);
    REQUIRE(tables[0].columns[0].name == "emp_id");
    REQUIRE(tables[0].columns[1].name == "salary");
}
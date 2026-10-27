#include <catch2/catch_test_macros.hpp>

#include "types/Common.hpp"
#include "types/Managers.hpp"

using namespace sqlweaver;

TEST_CASE("TableManager registers, deduplicates and merges tables", "[DomainModel]") {
    TableManager tm;

    Table read_table {
        "USERS",
        "PUBLIC",
        IoType::In,
        ActionType::Call,
        {SourceLocation{10, 5, 150}},
        {}
    };

    Table write_table {
        "users", // lowercase test
        "public",
        IoType::Out,
        ActionType::Call,
        {SourceLocation{25, 2, 300}},
        {}
    };

    tm.register_table(std::move(read_table));
    tm.register_table(std::move(write_table));

    // Deduplication check
    REQUIRE(tm.get_tables().size() == 1);
    
    // I/O merging check: In + Out = Both
    REQUIRE(tm.get_tables()[0].io_type == IoType::Both);
    
    // Locations check
    REQUIRE(tm.get_tables()[0].locations.size() == 2);

    tm.clear();
    REQUIRE(tm.get_tables().empty());
}

TEST_CASE("Procedure definition accepts parameters", "[DomainModel]") {
    Procedure proc {
        "CALCULATE_SALARY",
        std::nullopt,
        ActionType::Definition,
        {SourceLocation{1, 0, 0}},
        {
            Parameter{"EMP_ID", "NUMBER", std::nullopt, IoType::In, SourceLocation{1, 20, 20}}
        },
        {}
    };

    REQUIRE(proc.parameters.size() == 1);
    REQUIRE(proc.parameters[0].name == "EMP_ID");
}
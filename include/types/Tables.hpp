#pragma once

#include "Common.hpp"

#include <string>
#include <vector>
#include <optional>

namespace sqlweaver {

    /**
     * @brief Represents a column definition inside a CREATE TABLE statement.
     */
    struct TableColumn {
        std::string name;
        std::string datatype;
        std::optional<std::string> default_value;
        SourceLocation location;
    };

    /**
     * @brief Represents a relational database table (either defined or referenced).
     */
    struct Table {
        std::string name;
        std::optional<std::string> schema;
        
        IoType io_type{IoType::None};
        ActionType action{ActionType::Unknown};
        std::vector<SourceLocation> locations; // Aggregated list of all occurrences
        
        std::vector<TableColumn> columns; // Populated if action == ActionType::Definition
    };

} // namespace sqlweaver
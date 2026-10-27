#pragma once

#include "Common.hpp"

#include <string>
#include <vector>
#include <optional>

namespace sqlweaver {

    /**
     * @brief Represents a parameter in a procedure/function definition.
     */
    struct Parameter {
        std::string name;
        std::string datatype;
        std::optional<std::string> default_value;
        IoType io_type{IoType::In};
        SourceLocation location;
    };

    /**
     * @brief Represents an argument passed during a procedure call.
     */
    struct Argument {
        std::string value;
        SourceLocation location;
    };

    /**
     * @brief Represents a procedure or function (either defined or called).
     */
    struct Procedure {
        std::string name;
        std::optional<std::string> schema;
        
        ActionType action{ActionType::Unknown};
        std::vector<SourceLocation> locations; // Aggregated list of all occurrences
        
        std::vector<Parameter> parameters;
        std::vector<Argument> arguments;
    };

} // namespace sqlweaver
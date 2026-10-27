#pragma once

#include <string>
#include <cstddef>

namespace sqlweaver {

    struct SourceLocation {
        size_t line{0};
        size_t column{0};
        size_t offset{0};

        bool operator==(const SourceLocation& other) const = default; // C++20 default comparison operator
    };

    enum class IoType {
        None,
        In,
        Out,
        Both
    };

    enum class ActionType {
        Unknown,
        Definition,
        Call
    };

    struct ParseError {
        SourceLocation location;
        std::string message;
    };

    /**
     * @brief Severity level for static code analysis / linting rules.
     */
    enum class LintSeverity {
        Info,
        Warning,
        Critical
    };

    /**
     * @brief Represents a rule violation discovered by the static linter.
     */
    struct LintViolation {
        std::string rule_id;
        std::string message;
        LintSeverity severity;
        SourceLocation location;
    };

} // namespace sqlweaver
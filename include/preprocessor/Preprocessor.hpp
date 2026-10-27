#pragma once

#include <string>
#include <vector>
#include <cstddef>

namespace sqlweaver {

    /**
     * @brief Represents a single executable SQL/PLSQL block extracted from a script.
     */
    struct SqlBlock {
        std::string content;     // Cleaned SQL block content
        size_t start_line{1};    // 1-based line index where this block starts in the original file
        size_t start_offset{0};  // Absolute character offset in the original file
    };

    /**
     * @brief Preprocesses raw SQL text, handles line normalization,
     *        splits by '/' delimiters, and preserves accurate line number mapping.
     */
    class Preprocessor {
    public:
        Preprocessor() = default;

        /**
         * @brief Normalizes line endings and splits code into independent blocks.
         * @param raw_sql The raw SQL code from file or string.
         * @return Vector of SqlBlock structures with preserved source coordinates.
         */
        [[nodiscard]] std::vector<SqlBlock> process(const std::string& raw_sql) const;

    private:
        /**
         * @brief Normalizes carriage returns (\r\n -> \n, \r -> \n).
         */
        [[nodiscard]] std::string normalize_line_endings(const std::string& input) const;

        /**
         * @brief Checks if a line consists solely of a '/' delimiter (ignoring whitespace).
         */
        [[nodiscard]] bool is_delimiter_line(const std::string& line) const;
    };

} // namespace sqlweaver
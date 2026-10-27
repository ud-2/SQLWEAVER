#include "preprocessor/Preprocessor.hpp"

#include <sstream>
#include <algorithm>
#include <cctype>

namespace sqlweaver {

    std::string Preprocessor::normalize_line_endings(const std::string& input) const {
        std::string result;
        result.reserve(input.size());

        for (size_t i = 0; i < input.size(); ++i) {
            if (input[i] == '\r') {
                result.push_back('\n');
                if (i + 1 < input.size() && input[i + 1] == '\n') {
                    ++i; // Skip the \n in \r\n
                }
            } else {
                result.push_back(input[i]);
            }
        }
        return result;
    }

    bool Preprocessor::is_delimiter_line(const std::string& line) const {
        size_t first = line.find_first_not_of(" \t");
        if (first == std::string::npos) {
            return false;
        }
        size_t last = line.find_last_not_of(" \t");
        return (first == last && line[first] == '/');
    }

    std::vector<SqlBlock> Preprocessor::process(const std::string& raw_sql) const {
        std::vector<SqlBlock> blocks;
        std::string normalized = normalize_line_endings(raw_sql);

        std::istringstream stream(normalized);
        std::string line;

        std::string current_block_content;
        size_t current_line_num = 1;
        size_t block_start_line = 1;
        size_t block_start_offset = 0;
        size_t current_offset = 0;

        bool in_block = false;

        while (std::getline(stream, line)) {
            size_t line_length_with_newline = line.size() + 1; // +1 for the \n

            if (is_delimiter_line(line)) {
                // End of the current block
                if (in_block && !current_block_content.empty()) {
                    blocks.push_back(SqlBlock{
                        current_block_content,
                        block_start_line,
                        block_start_offset
                    });
                    current_block_content.clear();
                    in_block = false;
                }
            } else {
                // Check if this is the start of a new block (ignore leading blank lines)
                bool is_blank = line.find_first_not_of(" \t\n") == std::string::npos;

                if (!in_block && !is_blank) {
                    in_block = true;
                    block_start_line = current_line_num;
                    block_start_offset = current_offset;
                }

                if (in_block) {
                    current_block_content += line + "\n";
                }
            }

            current_offset += line_length_with_newline;
            current_line_num++;
        }

        // Handle any remaining block at the end of the file (if not terminated by '/')
        if (in_block && !current_block_content.empty()) {
            blocks.push_back(SqlBlock{
                current_block_content,
                block_start_line,
                block_start_offset
            });
        }

        return blocks;
    }

} // namespace sqlweaver
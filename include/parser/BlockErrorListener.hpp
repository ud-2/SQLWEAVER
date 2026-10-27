#pragma once

#include "antlr4-runtime.h"
#include "types/Common.hpp"

#include <vector>
#include <string>
#include <algorithm>

namespace sqlweaver {

    /**
     * @class BlockErrorListener
     * @brief Custom ANTLR error listener that sanitizes and maps line numbers.
     */
    class BlockErrorListener : public antlr4::BaseErrorListener {
    public:
        explicit BlockErrorListener(std::vector<ParseError>& errors, size_t line_offset = 1)
            : m_errors(errors), m_line_offset(line_offset > 0 ? line_offset - 1 : 0) {}

        void syntaxError(antlr4::Recognizer*,
                         antlr4::Token* /*offendingSymbol*/,
                         size_t line,
                         size_t charPositionInLine,
                         const std::string& msg,
                         std::exception_ptr /*e*/) override {
            
            // Clean newlines, carriage returns, and tabs
            std::string clean_msg = msg;
            std::replace(clean_msg.begin(), clean_msg.end(), '\n', ' ');
            std::replace(clean_msg.begin(), clean_msg.end(), '\r', ' ');
            std::replace(clean_msg.begin(), clean_msg.end(), '\t', ' ');

            // Truncate overly verbose ANTLR syntax dumps
            constexpr size_t MAX_MSG_LENGTH = 120;
            if (clean_msg.length() > MAX_MSG_LENGTH) {
                clean_msg = clean_msg.substr(0, MAX_MSG_LENGTH - 3) + "...";
            }

            m_errors.push_back(ParseError{
                SourceLocation{line + m_line_offset, charPositionInLine, 0},
                clean_msg
            });
        }

    private:
        std::vector<ParseError>& m_errors;
        size_t m_line_offset{0};
    };

} // namespace sqlweaver
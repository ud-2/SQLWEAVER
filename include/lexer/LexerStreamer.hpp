#pragma once

#include "lexer/CaseInsensitiveStream.hpp"
#include "lexer/PlSqlLexerBase.h"
#include "PlSqlLexer.h"
#include "antlr4-runtime.h"

#include <string>
#include <memory>

namespace sqlweaver {

    /**
     * @brief Holds the lifecycle of ANTLR stream objects to prevent dangling pointers.
     */
    struct TokenStreamResult {
        std::unique_ptr<CaseInsensitiveStream> stream;
        std::unique_ptr<PlSqlLexer> lexer;
        std::unique_ptr<antlr4::CommonTokenStream> tokens;
    };

    /**
     * @class LexerStreamer
     * @brief Encapsulates case-insensitive ANTLR lexing and token stream generation.
     */
    class LexerStreamer {
    public:
        LexerStreamer() = default;

        /**
         * @brief Converts SQL text into a populated ANTLR token stream.
         * @param sql_text The SQL block content.
         * @return TokenStreamResult containing the stream, lexer, and filled tokens.
         */
        [[nodiscard]] TokenStreamResult tokenize(const std::string& sql_text) const;
    };

} // namespace sqlweaver
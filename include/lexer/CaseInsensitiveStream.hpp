#pragma once

#include "antlr4-runtime.h"
#include <cctype>

namespace sqlweaver {

    /**
     * @brief Custom ANTLR input stream providing case-insensitive character lookahead.
     *
     * Overrides the lookahead (LA) method to convert characters to uppercase on-the-fly,
     * allowing the Lexer to match uppercase grammar keywords regardless of source case.
     */
    class CaseInsensitiveStream : public antlr4::ANTLRInputStream {
    public:
        using ANTLRInputStream::ANTLRInputStream;

        size_t LA(ssize_t i) override {
            size_t c = ANTLRInputStream::LA(i);

            // (c == 0) corresponds to the undefined case in LA original definition
            if (c == antlr4::CharStream::EOF || c == 0) {
                return c;
            }
            return static_cast<size_t>(std::toupper(static_cast<unsigned char>(c)));
        }
    };

} // namespace sqlweaver
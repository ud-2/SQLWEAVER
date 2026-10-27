#include "lexer/LexerStreamer.hpp"

namespace sqlweaver {

    TokenStreamResult LexerStreamer::tokenize(const std::string& sql_text) const {
        auto stream = std::make_unique<CaseInsensitiveStream>(sql_text);
        auto lexer = std::make_unique<PlSqlLexer>(stream.get());
        auto tokens = std::make_unique<antlr4::CommonTokenStream>(lexer.get());

        // Pre-fill all tokens from the stream
        tokens->fill();

        return TokenStreamResult{
            std::move(stream),
            std::move(lexer),
            std::move(tokens)
        };
    }

} // namespace sqlweaver
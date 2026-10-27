#include "parser/ParserWrapper.hpp"

namespace sqlweaver {

    ParseResult ParserWrapper::parse(antlr4::CommonTokenStream* tokens, size_t start_line) const {
        ParseResult result;

        result.parser = std::make_unique<PlSqlParser>(tokens);
        result.parser->setVersion12(true);

        // Configure error strategy for recovery
        result.parser->setErrorHandler(std::make_shared<antlr4::DefaultErrorStrategy>());

        // Attach custom error listener with source line offset
        BlockErrorListener error_listener(result.errors, start_line);
        result.parser->removeErrorListeners();
        result.parser->addErrorListener(&error_listener);

        try {
            // Parse main entry rule for PL/SQL scripts
            result.ast = result.parser->sql_script();
        } catch (const std::exception& e) {
            result.errors.push_back(ParseError{
                SourceLocation{start_line, 0, 0},
                e.what()
            });
        } catch (...) {
            result.errors.push_back(ParseError{
                SourceLocation{start_line, 0, 0},
                "Unknown critical failure during parsing"
            });
        }

        return result;
    }

} // namespace sqlweaver
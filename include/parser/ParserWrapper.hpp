#pragma once

#include "parser/PlSqlParserBase.h"
#include "PlSqlParser.h"
#include "parser/BlockErrorListener.hpp"
#include "types/Common.hpp"

#include <vector>
#include <memory>

namespace sqlweaver {

    /**
     * @brief Holds the parse tree root node, parser lifecycle, and accumulated errors.
     */
    struct ParseResult {
        PlSqlParser::Sql_scriptContext* ast{nullptr};
        std::unique_ptr<PlSqlParser> parser;
        std::vector<ParseError> errors;
    };

    /**
     * @class ParserWrapper
     * @brief Encapsulates ANTLR PlSqlParser initialization, error listener setup, and AST generation.
     */
    class ParserWrapper {
    public:
        ParserWrapper() = default;

        /**
         * @brief Parses an ANTLR CommonTokenStream into an AST root node.
         * @param tokens Pointer to populated token stream.
         * @param start_line 1-based line number offset of the block in the source file.
         * @return ParseResult containing the AST, parser instance, and syntax errors.
         */
        [[nodiscard]] ParseResult parse(antlr4::CommonTokenStream* tokens, size_t start_line = 1) const;
    };

} // namespace sqlweaver
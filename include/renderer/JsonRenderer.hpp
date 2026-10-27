#pragma once

#include "renderer/IRenderer.hpp"

namespace sqlweaver {

    /**
     * @class JsonRenderer
     * @brief Exports the entire AST extraction and linting results as a structured JSON document.
     */
    class JsonRenderer : public IRenderer {
    public:
        JsonRenderer() = default;

        [[nodiscard]] std::string render(const TableManager& tm,
                                         const ProcedureManager& pm,
                                         const std::vector<ParseError>& errors,
                                         const std::vector<LintViolation>& violations) const override;
    };

} // namespace sqlweaver
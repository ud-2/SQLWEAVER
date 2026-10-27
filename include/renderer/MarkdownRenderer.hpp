#pragma once

#include "renderer/IRenderer.hpp"

namespace sqlweaver {

    /**
     * @class MarkdownRenderer
     * @brief Formats extracted SQL metadata into a structured Markdown (.md) document.
     */
    class MarkdownRenderer : public IRenderer {
    public:
        MarkdownRenderer() = default;

        [[nodiscard]] std::string render(const TableManager& tm,
                                                 const ProcedureManager& pm,
                                                 const std::vector<ParseError>& errors,
                                                 const std::vector<LintViolation>& violations) const override;
    };

} // namespace sqlweaver
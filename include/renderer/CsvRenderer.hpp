#pragma once

#include "renderer/IRenderer.hpp"

namespace sqlweaver {

    class CsvRenderer : public IRenderer {
    public:
        CsvRenderer() = default;

        [[nodiscard]] std::string render(const TableManager& tm,
                                         const ProcedureManager& pm,
                                         const std::vector<ParseError>& errors,
                                         const std::vector<LintViolation>& violations) const override;
    };

} // namespace sqlweaver
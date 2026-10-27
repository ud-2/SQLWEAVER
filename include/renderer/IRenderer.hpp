#pragma once

#include "types/Managers.hpp"
#include "types/Common.hpp"

#include <string>
#include <vector>

namespace sqlweaver {

    class IRenderer {
    public:
        virtual ~IRenderer() = default;

        [[nodiscard]] virtual std::string render(const TableManager& tm,
                                                 const ProcedureManager& pm,
                                                 const std::vector<ParseError>& errors,
                                                 const std::vector<LintViolation>& violations) const = 0;
    };

} // namespace sqlweaver
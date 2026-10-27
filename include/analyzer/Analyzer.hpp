#pragma once

#include "types/Common.hpp"
#include "types/Managers.hpp"
#include "renderer/IRenderer.hpp"

#include <string>
#include <vector>
#include <memory>

namespace sqlweaver {

    class Analyzer {
    public:
        Analyzer();
        ~Analyzer();

        Analyzer(Analyzer&&) noexcept;
        Analyzer& operator=(Analyzer&&) noexcept;

        Analyzer(const Analyzer&) = delete;
        Analyzer& operator=(const Analyzer&) = delete;

        bool analyze_string(const std::string& sql_content);
        bool analyze_file(const std::string& filepath);

        /* The return values of these functions should not be ignored. */

        [[nodiscard]] std::string render_report(const IRenderer& renderer) const;

        [[nodiscard]] const TableManager& get_table_manager() const noexcept;
        [[nodiscard]] const ProcedureManager& get_procedure_manager() const noexcept;
        [[nodiscard]] const std::vector<ParseError>& get_errors() const noexcept;
        [[nodiscard]] const std::vector<LintViolation>& get_violations() const noexcept;
        [[nodiscard]] bool has_critical_violations() const noexcept;

    // For friendly API usage
    // We can make the implementation details private and use the Pimpl idiom to hide them from the public interface.
    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };

} // namespace sqlweaver
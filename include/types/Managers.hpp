#pragma once

#include "Tables.hpp"
#include "Procedures.hpp"

#include <vector>
#include <algorithm>
#include <cctype>

namespace sqlweaver {

    namespace detail {
        inline std::string to_upper(std::string_view s) {
            std::string result;
            result.reserve(s.size());

            for (char c : s) {
                result.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
            }

            return result;
        }

        inline bool iequals(const std::optional<std::string>& a, const std::optional<std::string>& b) {
            if (a.has_value() != b.has_value()) return false;
            if (!a.has_value()) return true;
            return to_upper(*a) == to_upper(*b);
        }
    }

    /**
     * @brief Stores, deduplicates, and manages extracted database tables.
     */
    class TableManager {
    public:
        void register_table(Table&& table) {
            auto it = std::find_if(m_tables.begin(), m_tables.end(), [&](const Table& t) {
                return detail::to_upper(t.name) == detail::to_upper(table.name) &&
                       detail::iequals(t.schema, table.schema);
            });

            if (it != m_tables.end()) {
                // Merge I/O contexts: In + Out = Both
                if ((it->io_type == IoType::In && table.io_type == IoType::Out) ||
                    (it->io_type == IoType::Out && table.io_type == IoType::In) ||
                    table.io_type == IoType::Both) {
                    it->io_type = IoType::Both;
                } else if (it->io_type == IoType::None) {
                    it->io_type = table.io_type;
                }

                // Definition takes priority over Call
                if (table.action == ActionType::Definition) {
                    it->action = ActionType::Definition;
                    if (!table.columns.empty()) {
                        it->columns = std::move(table.columns);
                    }
                }

                // Append unique occurrences
                for (const auto& loc : table.locations) {
                    if (std::find(it->locations.begin(), it->locations.end(), loc) == it->locations.end()) {
                        it->locations.push_back(loc);
                    }
                }
            } else {
                m_tables.push_back(std::move(table));
            }
        }

        void clear() noexcept {
            m_tables.clear();
        }

        [[nodiscard]] const std::vector<Table>& get_tables() const noexcept {
            return m_tables;
        }

    private:
        std::vector<Table> m_tables;
    };

    /**
     * @brief Stores, deduplicates, and manages extracted procedures.
     */
    class ProcedureManager {
    public:
        void register_procedure(Procedure&& procedure) {
            auto it = std::find_if(m_procedures.begin(), m_procedures.end(), [&](const Procedure& p) {
                return detail::to_upper(p.name) == detail::to_upper(procedure.name) &&
                       detail::iequals(p.schema, procedure.schema);
            });

            if (it != m_procedures.end()) {
                if (procedure.action == ActionType::Definition) {
                    it->action = ActionType::Definition;
                    if (!procedure.parameters.empty()) {
                        it->parameters = std::move(procedure.parameters);
                    }
                }

                for (const auto& loc : procedure.locations) {
                    if (std::find(it->locations.begin(), it->locations.end(), loc) == it->locations.end()) {
                        it->locations.push_back(loc);
                    }
                }
            } else {
                m_procedures.push_back(std::move(procedure));
            }
        }

        void clear() noexcept {
            m_procedures.clear();
        }

        [[nodiscard]] const std::vector<Procedure>& get_procedures() const noexcept {
            return m_procedures;
        }

    private:
        std::vector<Procedure> m_procedures;
    };

} // namespace sqlweaver
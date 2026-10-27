#include "renderer/MarkdownRenderer.hpp"
#include <sstream>

namespace sqlweaver {

    static std::string io_type_to_string(IoType type) {
        switch (type) {
            case IoType::In: return "READ (In)";
            case IoType::Out: return "WRITE (Out)";
            case IoType::Both: return "READ/WRITE (Both)";
            default: return "NONE";
        }
    }

    static std::string action_type_to_string(ActionType action) {
        switch (action) {
            case ActionType::Definition: return "DEFINITION";
            case ActionType::Call: return "CALL";
            default: return "UNKNOWN";
        }
    }

    std::string MarkdownRenderer::render(const TableManager& tm,
                                         const ProcedureManager& pm,
                                         const std::vector<ParseError>& errors,
                                         const std::vector<LintViolation>& violations) const {
        std::ostringstream oss;

        const auto& tables = tm.get_tables();
        const auto& procedures = pm.get_procedures();

        oss << "# SQLWeaver Analysis Report\n\n";

        // Summary Section
        oss << "## Summary\n";
        oss << "- **Unique Tables/Views Extracted:** " << tables.size() << "\n";
        oss << "- **Unique Procedures Extracted:** " << procedures.size() << "\n";
        oss << "- **Quality / Security Violations:** " << violations.size() << "\n";
        oss << "- **Syntax Errors / Warnings:** " << errors.size() << "\n\n";

        // Linter Violations
        if (!violations.empty()) {
            oss << "## Quality & Security Violations\n\n";
            for (const auto& v : violations) {
                std::string badge = (v.severity == LintSeverity::Critical) ? "CRITICAL" : "WARNING";
                oss << "- **[" << badge << "] `" << v.rule_id << "` (Line " << v.location.line << "):** "
                    << v.message << "\n";
            }
            oss << "\n";
        }

        // Tables Section
        oss << "## Tables & Views\n\n";
        if (tables.empty()) {
            oss << "*No tables found.*\n\n";
        } else {
            for (const auto& table : tables) {
                oss << "### Table: `"
                    << (table.schema ? *table.schema + "." : "")
                    << table.name << "`\n";
                oss << "- **Action:** " << action_type_to_string(table.action) << "\n";
                oss << "- **I/O Type:** " << io_type_to_string(table.io_type) << "\n";
                
                oss << "- **Occurrences (" << table.locations.size() << "):** ";
                for (size_t i = 0; i < table.locations.size(); ++i) {
                    if (i > 0) oss << ", ";
                    oss << "Line " << table.locations[i].line;
                }
                oss << "\n\n";
            }
        }

        // Procedures Section
        oss << "## Procedures & Routines\n\n";
        if (procedures.empty()) {
            oss << "*No procedures found.*\n\n";
        } else {
            for (const auto& proc : procedures) {
                oss << "### Procedure: `" 
                    << (proc.schema ? *proc.schema + "." : "") 
                    << proc.name << "`\n";
                oss << "- **Action:** " << action_type_to_string(proc.action) << "\n";
                
                oss << "- **Occurrences (" << proc.locations.size() << "):** ";
                for (size_t i = 0; i < proc.locations.size(); ++i) {
                    if (i > 0) oss << ", ";
                    oss << "Line " << proc.locations[i].line;
                }
                oss << "\n\n";
            }
        }

        // Errors Section
        if (!errors.empty()) {
            oss << "## Syntax Warnings & Errors\n\n";
            for (const auto& err : errors) {
                oss << "- **Line " << err.location.line << ", Col " << err.location.column << ":** " 
                    << err.message << "\n";
            }
            oss << "\n";
        }

        return oss.str();
    }

} // namespace sqlweaver
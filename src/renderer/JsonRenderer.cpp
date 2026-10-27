#include "renderer/JsonRenderer.hpp"
#include <sstream>
#include <iomanip>

namespace sqlweaver {

    static std::string escape_json(std::string_view s) {
        std::ostringstream oss;
        for (char c : s) {
            switch (c) {
                case '"': oss << "\\\""; break;
                case '\\': oss << "\\\\"; break;
                case '\b': oss << "\\b"; break;
                case '\f': oss << "\\f"; break;
                case '\n': oss << "\\n"; break;
                case '\r': oss << "\\r"; break;
                case '\t': oss << "\\t"; break;
                default:
                    if ('\x00' <= c && c <= '\x1f') {
                        oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                    } else {
                        oss << c;
                    }
            }
        }
        return oss.str();
    }

    static std::string severity_to_string(LintSeverity s) {
        switch (s) {
            case LintSeverity::Critical: return "CRITICAL";
            case LintSeverity::Warning: return "WARNING";
            default: return "INFO";
        }
    }

    static std::string io_type_to_string(IoType type) {
        switch (type) {
            case IoType::In: return "READ";
            case IoType::Out: return "WRITE";
            case IoType::Both: return "READ_WRITE";
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

    std::string JsonRenderer::render(const TableManager& tm,
                                     const ProcedureManager& pm,
                                     const std::vector<ParseError>& errors,
                                     const std::vector<LintViolation>& violations) const {
        std::ostringstream json;
        const auto& tables = tm.get_tables();
        const auto& procedures = pm.get_procedures();

        json << "{\n";
        
        // Summary
        json << "  \"summary\": {\n";
        json << "    \"tables_count\": " << tables.size() << ",\n";
        json << "    \"procedures_count\": " << procedures.size() << ",\n";
        json << "    \"errors_count\": " << errors.size() << ",\n";
        json << "    \"violations_count\": " << violations.size() << "\n";
        json << "  },\n";

        // Linter Violations
        json << "  \"violations\": [\n";
        for (size_t i = 0; i < violations.size(); ++i) {
            const auto& v = violations[i];
            json << "    {\n";
            json << "      \"rule_id\": \"" << escape_json(v.rule_id) << "\",\n";
            json << "      \"severity\": \"" << severity_to_string(v.severity) << "\",\n";
            json << "      \"message\": \"" << escape_json(v.message) << "\",\n";
            json << "      \"line\": " << v.location.line << ",\n";
            json << "      \"column\": " << v.location.column << "\n";
            json << "    }" << (i + 1 < violations.size() ? "," : "") << "\n";
        }
        json << "  ],\n";

        // Tables
        json << "  \"tables\": [\n";
        for (size_t i = 0; i < tables.size(); ++i) {
            const auto& t = tables[i];
            json << "    {\n";
            json << "      \"name\": \"" << escape_json(t.name) << "\",\n";
            json << "      \"schema\": " << (t.schema ? "\"" + escape_json(*t.schema) + "\"" : "null") << ",\n";
            json << "      \"io_type\": \"" << io_type_to_string(t.io_type) << "\",\n";
            json << "      \"action\": \"" << action_type_to_string(t.action) << "\",\n";
            
            // Locations
            json << "      \"locations\": [";
            for (size_t j = 0; j < t.locations.size(); ++j) {
                json << "{\"line\": " << t.locations[j].line << ", \"col\": " << t.locations[j].column << "}"
                     << (j + 1 < t.locations.size() ? ", " : "");
            }
            json << "],\n";

            // Columns
            json << "      \"columns\": [";
            for (size_t j = 0; j < t.columns.size(); ++j) {
                const auto& col = t.columns[j];
                json << "{\"name\": \"" << escape_json(col.name) << "\", \"type\": \"" << escape_json(col.datatype) << "\"}";
                if (j + 1 < t.columns.size()) json << ", ";
            }
            json << "]\n";
            json << "    }" << (i + 1 < tables.size() ? "," : "") << "\n";
        }
        json << "  ],\n";

        // Procedures
        json << "  \"procedures\": [\n";
        for (size_t i = 0; i < procedures.size(); ++i) {
            const auto& p = procedures[i];
            json << "    {\n";
            json << "      \"name\": \"" << escape_json(p.name) << "\",\n";
            json << "      \"schema\": " << (p.schema ? "\"" + escape_json(*p.schema) + "\"" : "null") << ",\n";
            json << "      \"action\": \"" << action_type_to_string(p.action) << "\",\n";
            
            // Locations
            json << "      \"locations\": [";
            for (size_t j = 0; j < p.locations.size(); ++j) {
                json << "{\"line\": " << p.locations[j].line << ", \"col\": " << p.locations[j].column << "}"
                     << (j + 1 < p.locations.size() ? ", " : "");
            }
            json << "],\n";

            // Parameters
            json << "      \"parameters\": [";
            for (size_t j = 0; j < p.parameters.size(); ++j) {
                const auto& param = p.parameters[j];
                json << "{\"name\": \"" << escape_json(param.name) << "\", \"type\": \"" << escape_json(param.datatype) << "\"}";
                if (j + 1 < p.parameters.size()) json << ", ";
            }
            json << "]\n";
            json << "    }" << (i + 1 < procedures.size() ? "," : "") << "\n";
        }
        json << "  ],\n";

        // Errors
        json << "  \"errors\": [\n";
        for (size_t i = 0; i < errors.size(); ++i) {
            const auto& err = errors[i];
            json << "    {\n";
            json << "      \"line\": " << err.location.line << ",\n";
            json << "      \"column\": " << err.location.column << ",\n";
            json << "      \"message\": \"" << escape_json(err.message) << "\"\n";
            json << "    }" << (i + 1 < errors.size() ? "," : "") << "\n";
        }
        json << "  ]\n";
        json << "}\n";

        return json.str();
    }

} // namespace sqlweaver
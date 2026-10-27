#include "renderer/CsvRenderer.hpp"
#include <sstream>

namespace sqlweaver {

    std::string CsvRenderer::render(const TableManager& tm,
                                    const ProcedureManager& pm,
                                    const std::vector<ParseError>& /*errors*/,
                                    const std::vector<LintViolation>& /*violations*/) const {
        std::ostringstream csv;
        csv << "ENTITY_TYPE,SCHEMA,NAME,ACTION,IO_TYPE,LINE,COLUMN\n";

        for (const auto& t : tm.get_tables()) {
            for (const auto& loc : t.locations) {
                csv << "TABLE,\"" 
                    << (t.schema ? *t.schema : "") << "\",\""
                    << t.name << "\",\""
                    << (t.action == ActionType::Definition ? "DEFINITION" : "CALL") << "\",\""
                    << (t.io_type == IoType::In ? "READ" : (t.io_type == IoType::Out ? "WRITE" : "BOTH")) << "\","
                    << loc.line << ","
                    << loc.column << "\n";
            }
        }

        for (const auto& p : pm.get_procedures()) {
            for (const auto& loc : p.locations) {
                csv << "PROCEDURE,\"" 
                    << (p.schema ? *p.schema : "") << "\",\""
                    << p.name << "\",\""
                    << (p.action == ActionType::Definition ? "DEFINITION" : "CALL") << "\",\""
                    << "EXECUTE\","
                    << loc.line << ","
                    << loc.column << "\n";
            }
        }

        return csv.str();
    }

} // namespace sqlweaver
#pragma once

#include "parser/PlSqlParserBase.h"
#include "PlSqlParserBaseVisitor.h"
#include "PlSqlParser.h"
#include "types/Managers.hpp"
#include "types/Common.hpp"

#include <optional>
#include <string>
#include <utility>
#include <any>
#include <unordered_set>
#include <vector>

namespace sqlweaver {

    class PlSqlVisitor : public PlSqlParserBaseVisitor {
    public:
        PlSqlVisitor(TableManager& tm, ProcedureManager& pm, size_t line_offset = 1)
            : m_tm(tm), m_pm(pm), m_line_offset(line_offset > 0 ? line_offset - 1 : 0) {}

        // Context Handling (DML & Subqueries)
        std::any visitSelect_statement(PlSqlParser::Select_statementContext* ctx) override;
        std::any visitInsert_statement(PlSqlParser::Insert_statementContext* ctx) override;
        std::any visitUpdate_statement(PlSqlParser::Update_statementContext* ctx) override;
        std::any visitDelete_statement(PlSqlParser::Delete_statementContext* ctx) override;
        std::any visitSubquery(PlSqlParser::SubqueryContext* ctx) override;

        // Linter Rules
        std::any visitSelected_list(PlSqlParser::Selected_listContext* ctx) override;

        // CTE & Grants
        std::any visitSubquery_factoring_clause(PlSqlParser::Subquery_factoring_clauseContext* ctx) override;
        std::any visitGrant_statement(PlSqlParser::Grant_statementContext* ctx) override;

        // Entity Extraction
        std::any visitCreate_table(PlSqlParser::Create_tableContext* ctx) override;
        std::any visitTableview_name(PlSqlParser::Tableview_nameContext* ctx) override;
        std::any visitCreate_procedure_body(PlSqlParser::Create_procedure_bodyContext* ctx) override;
        std::any visitCall_statement(PlSqlParser::Call_statementContext* ctx) override;
        std::any visitGeneral_element(PlSqlParser::General_elementContext* ctx) override;

        std::any visitExecute_immediate(PlSqlParser::Execute_immediateContext* ctx) override;
        std::any visitException_handler(PlSqlParser::Exception_handlerContext* ctx) override;

        [[nodiscard]] const std::vector<LintViolation>& get_violations() const noexcept { return m_violations; }

    private:
        TableManager& m_tm;
        ProcedureManager& m_pm;
        size_t m_line_offset{0};
        IoType m_current_io_type{IoType::In};
        std::unordered_set<std::string> m_cte_names;
        std::vector<LintViolation> m_violations;

        std::any _set_context_and_visit(antlr4::ParserRuleContext* ctx, IoType new_io_type);
        [[nodiscard]] std::pair<std::optional<std::string>, std::string> _extract_schema_and_name(const std::string& full_name) const;
        [[nodiscard]] SourceLocation _make_location(antlr4::Token* token) const noexcept;
        [[nodiscard]] bool _is_builtin_function(std::string_view name) const noexcept;
    };

} // namespace sqlweaver
#include "visitor/PlSqlVisitor.hpp"

#include <algorithm>
#include <sstream>

namespace sqlweaver {

    // Context Handling & Linter Rules for DMLs

    std::any PlSqlVisitor::visitSelect_statement(PlSqlParser::Select_statementContext* ctx) {
        return _set_context_and_visit(ctx, IoType::In);
    }

    std::any PlSqlVisitor::visitSelected_list(PlSqlParser::Selected_listContext* ctx) {
        if (!ctx) return nullptr;

        // LINTER RULE 1: Detect SELECT *
        for (auto* elem : ctx->select_list_elements()) {
            if (elem && elem->getText() == "*") {
                m_violations.push_back(LintViolation{
                    "PERF-001",
                    "Use of 'SELECT *' is discouraged in production. Explicitly specify column names.",
                    LintSeverity::Warning,
                    _make_location(elem->getStart())
                });
            }
        }
        return visitChildren(ctx);
    }

    std::any PlSqlVisitor::visitInsert_statement(PlSqlParser::Insert_statementContext* ctx) {
        return _set_context_and_visit(ctx, IoType::Out);
    }

    std::any PlSqlVisitor::visitUpdate_statement(PlSqlParser::Update_statementContext* ctx) {
        if (!ctx) return nullptr;

        // LINTER RULE 2: Detect UPDATE without WHERE clause
        if (!ctx->where_clause()) {
            m_violations.push_back(LintViolation{
                "SEC-001",
                "Dangerous UPDATE statement without WHERE clause detected (Affects all table rows).",
                LintSeverity::Critical,
                _make_location(ctx->getStart())
            });
        }
        return _set_context_and_visit(ctx, IoType::Out);
    }

    std::any PlSqlVisitor::visitDelete_statement(PlSqlParser::Delete_statementContext* ctx) {
        if (!ctx) return nullptr;

        // LINTER RULE 3: Detect DELETE without WHERE clause
        if (!ctx->where_clause()) {
            m_violations.push_back(LintViolation{
                "SEC-002",
                "Dangerous DELETE statement without WHERE clause detected (Deletes entire table content).",
                LintSeverity::Critical,
                _make_location(ctx->getStart())
            });
        }
        return _set_context_and_visit(ctx, IoType::Out);
    }

    std::any PlSqlVisitor::visitSubquery(PlSqlParser::SubqueryContext* ctx) {
        return _set_context_and_visit(ctx, IoType::In);
    }

    std::any PlSqlVisitor::_set_context_and_visit(antlr4::ParserRuleContext* ctx, IoType new_io_type) {
        if (!ctx) return nullptr;
        IoType previous = m_current_io_type;
        m_current_io_type = new_io_type;
        auto result = visitChildren(ctx);
        m_current_io_type = previous;
        return result;
    }

    // CTE & Grant Handling

    std::any PlSqlVisitor::visitSubquery_factoring_clause(PlSqlParser::Subquery_factoring_clauseContext* ctx) {
        if (ctx && ctx->query_name()) {
            std::string cte_name = ctx->query_name()->getText();
            std::transform(cte_name.begin(), cte_name.end(), cte_name.begin(), ::toupper);
            m_cte_names.insert(cte_name);
        }
        return visitChildren(ctx);
    }

    std::any PlSqlVisitor::visitGrant_statement(PlSqlParser::Grant_statementContext* /*ctx*/) {
        return nullptr;
    }

    // Entity Extraction

    std::any PlSqlVisitor::visitCreate_table(PlSqlParser::Create_tableContext* ctx) {
        if (!ctx) return nullptr;

        auto table_name_ctx = ctx->table_name();
        if (!table_name_ctx) return visitChildren(ctx);

        std::string name = table_name_ctx->getText();
        name.erase(std::remove(name.begin(), name.end(), '"'), name.end());

        std::optional<std::string> schema;
        if (ctx->schema_name()) {
            std::string schema_str = ctx->schema_name()->getText();
            schema_str.erase(std::remove(schema_str.begin(), schema_str.end(), '"'), schema_str.end());
            schema = schema_str;
        }

        std::vector<TableColumn> columns;
        auto relational_table = ctx->relational_table();
        if (relational_table) {
            for (auto property : relational_table->relational_property()) {
                if (!property) continue;
                auto column_def = property->column_definition();
                if (!column_def) continue;

                auto col_name_ctx = column_def->column_name();
                auto datatype_ctx = column_def->datatype();
                if (!col_name_ctx || !datatype_ctx) continue;

                auto expr_ctx = column_def->expression();
                std::optional<std::string> default_val = (expr_ctx && column_def->DEFAULT() != nullptr)
                    ? std::optional<std::string>(expr_ctx->getText())
                    : std::nullopt;

                columns.push_back(TableColumn{
                    col_name_ctx->getText(),
                    datatype_ctx->getText(),
                    default_val,
                    _make_location(column_def->getStart())
                });
            }
        }

        m_tm.register_table(Table{
            name,
            schema,
            IoType::None,
            ActionType::Definition,
            {_make_location(ctx->getStart())},
            std::move(columns)
        });

        return nullptr;
    }

    std::any PlSqlVisitor::visitTableview_name(PlSqlParser::Tableview_nameContext* ctx) {
        if (!ctx) return nullptr;

        auto [schema, name] = _extract_schema_and_name(ctx->getText());
        if (name.empty()) return nullptr;

        std::string upper_name = name;
        std::transform(upper_name.begin(), upper_name.end(), upper_name.begin(), ::toupper);
        if (m_cte_names.count(upper_name)) {
            return nullptr;
        }

        m_tm.register_table(Table{
            name,
            schema,
            m_current_io_type,
            ActionType::Call,
            {_make_location(ctx->getStart())},
            {}
        });

        return nullptr;
    }

    std::any PlSqlVisitor::visitCreate_procedure_body(PlSqlParser::Create_procedure_bodyContext* ctx) {
        if (!ctx || !ctx->procedure_name()) return nullptr;

        std::vector<Parameter> parameters;
        for (auto parameter : ctx->parameter()) {
            if (!parameter || !parameter->parameter_name() || !parameter->type_spec()) continue;

            IoType io_type = IoType::In;
            if (!parameter->OUT().empty()) {
                io_type = parameter->IN().empty() ? IoType::Out : IoType::Both;
            }

            auto default_part = parameter->default_value_part();
            std::optional<std::string> default_val = default_part 
                ? std::optional<std::string>(default_part->getText()) 
                : std::nullopt;

            parameters.push_back(Parameter{
                parameter->parameter_name()->getText(),
                parameter->type_spec()->getText(),
                default_val,
                io_type,
                _make_location(parameter->getStart())
            });
        }

        auto [schema, name] = _extract_schema_and_name(ctx->procedure_name()->getText());

        m_pm.register_procedure(Procedure{
            name,
            schema,
            ActionType::Definition,
            {_make_location(ctx->getStart())},
            std::move(parameters),
            {}
        });

        return visitChildren(ctx);
    }

    std::any PlSqlVisitor::visitCall_statement(PlSqlParser::Call_statementContext* ctx) {
        if (!ctx) return nullptr;

        auto routine_names = ctx->routine_name();
        if (routine_names.empty()) return visitChildren(ctx);

        std::optional<std::string> schema;
        if (routine_names.size() > 1) {
            if (routine_names[0]) schema = routine_names[0]->getText();
            routine_names.erase(routine_names.begin());
        }

        std::ostringstream oss;
        for (size_t i = 0; i < routine_names.size(); ++i) {
            if (!routine_names[i]) continue;
            if (i > 0) oss << ".";
            oss << routine_names[i]->getText();
        }

        std::vector<Argument> args;
        for (auto arg : ctx->function_argument()) {
            if (arg) {
                args.push_back(Argument{
                    arg->getText(),
                    _make_location(arg->getStart())
                });
            }
        }

        std::string full_name = oss.str();
        if (!_is_builtin_function(full_name)) {
            m_pm.register_procedure(Procedure{
                full_name,
                schema,
                ActionType::Call,
                {_make_location(ctx->getStart())},
                {},
                std::move(args)
            });
        }

        return nullptr;
    }

    std::any PlSqlVisitor::visitGeneral_element(PlSqlParser::General_elementContext* ctx) {
        if (!ctx) return nullptr;

        for (auto* call : ctx->general_element_part()) {
            if (!call || !call->id_expression()) continue;

            auto arguments = call->function_argument();
            if (arguments.empty()) continue;

            auto [schema, name] = _extract_schema_and_name(call->id_expression()->getText());
            if (name.empty()) continue;

            if (_is_builtin_function(name)) continue;

            std::vector<Argument> args;
            for (auto* arg : arguments) {
                if (arg) {
                    args.push_back(Argument{
                        arg->getText(),
                        _make_location(arg->getStart())
                    });
                }
            }

            m_pm.register_procedure(Procedure{
                name,
                schema,
                ActionType::Call,
                {_make_location(call->getStart())},
                {},
                std::move(args)
            });
        }

        return visitChildren(ctx);
    }

    // Private Helper Methods

    bool PlSqlVisitor::_is_builtin_function(std::string_view name) const noexcept {
        static const std::unordered_set<std::string_view> oracle_builtins = {
            "SUBSTR", "NVL", "NVL2", "TO_CHAR", "TO_DATE", "TO_NUMBER",
            "LAST_DAY", "LPAD", "RPAD", "CONCAT", "LENGTH", "TRUNC", "ROUND",
            "DECODE", "COALESCE", "NULLIF", "GREATEST", "LEAST", "ABS",
            "MOD", "UPPER", "LOWER", "INITCAP", "TRIM", "LTRIM", "RTRIM",
            "REPLACE", "INSTR", "ADD_MONTHS", "MONTHS_BETWEEN", "SYSDATE",
            "EXTRACT", "COUNT", "SUM", "AVG", "MIN", "MAX"
        };

        std::string upper_name;
        upper_name.reserve(name.size());
        for (char c : name) {
            upper_name.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
        }

        return oracle_builtins.count(upper_name) > 0;
    }

    SourceLocation PlSqlVisitor::_make_location(antlr4::Token* token) const noexcept {
        if (!token) return SourceLocation{1, 0, 0};
        return SourceLocation{
            token->getLine() + m_line_offset,
            token->getCharPositionInLine(),
            token->getStartIndex()
        };
    }

    std::pair<std::optional<std::string>, std::string> PlSqlVisitor::_extract_schema_and_name(const std::string& full_name) const {
        if (full_name.empty()) return {std::nullopt, ""};

        std::string identifier = full_name;
        std::optional<std::string> schema;
        std::string name;

        identifier.erase(std::remove(identifier.begin(), identifier.end(), '"'), identifier.end());

        size_t dot_pos = identifier.find('.');
        if (dot_pos != std::string::npos) {
            schema = identifier.substr(0, dot_pos);
            name = identifier.substr(dot_pos + 1);
        } else {
            name = identifier;
        }

        return {schema, name};
    }

    std::any PlSqlVisitor::visitExecute_immediate(PlSqlParser::Execute_immediateContext* ctx) {
        if (!ctx) return nullptr;

        // Inspect dynamic SQL expressions for suspicious concatenations
        // (presence of '||' indicating variable concatenation instead of pure literals).
        if (ctx->expression()) {
            std::string dynamic_sql = ctx->expression()->getText();
            bool has_concat = dynamic_sql.find("||") != std::string::npos;

            if (has_concat) {
                m_violations.push_back(LintViolation{
                    "SEC-003",
                    "EXECUTE IMMEDIATE with string concatenation detected: "
                    "potential SQL injection vulnerability. Use bind variables (USING clause) instead.",
                    LintSeverity::Critical,
                    _make_location(ctx->getStart())
                });
            }
        }

        return visitChildren(ctx);
    }

    std::any PlSqlVisitor::visitException_handler(PlSqlParser::Exception_handlerContext* ctx) {
        if (!ctx) return nullptr;

        // Check if the handler catches "OTHERS"
        bool catches_others = false;
        for (auto* exc_name : ctx->exception_name()) {
            if (exc_name) {
                std::string name = exc_name->getText();
                std::transform(name.begin(), name.end(), name.begin(), ::toupper);
                if (name == "OTHERS") {
                    catches_others = true;
                    break;
                }
            }
        }

        if (catches_others) {
            // Check if the handler body contains only NULL or is empty
            std::string body_text = ctx->seq_of_statements()
                ? ctx->seq_of_statements()->getText()
                : "";

            // Normalize to upper-case to catch variations like "null;" or "NULL;"
            std::transform(body_text.begin(), body_text.end(), body_text.begin(), ::toupper);

            if (body_text == "NULL;" || body_text.empty()) {
                m_violations.push_back(LintViolation{
                    "QUAL-001",
                    "WHEN OTHERS THEN NULL detected: exceptions are silently swallowed, "
                    "masking potential runtime errors.",
                    LintSeverity::Warning,
                    _make_location(ctx->getStart())
                });
            }
        }

        return visitChildren(ctx);
    }

} // namespace sqlweaver
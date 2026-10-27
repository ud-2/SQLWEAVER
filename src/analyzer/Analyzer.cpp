#include "analyzer/Analyzer.hpp"

#include "preprocessor/Preprocessor.hpp"
#include "lexer/LexerStreamer.hpp"
#include "parser/ParserWrapper.hpp"
#include "visitor/PlSqlVisitor.hpp"

#include <fstream>
#include <sstream>

namespace sqlweaver {

    struct Analyzer::Impl {
        TableManager table_manager;
        ProcedureManager procedure_manager;
        std::vector<ParseError> errors;
        std::vector<LintViolation> violations;

        Preprocessor preprocessor;
        LexerStreamer lexer_streamer;
        ParserWrapper parser_wrapper;

        void clear() {
            table_manager.clear();
            procedure_manager.clear();
            errors.clear();
            violations.clear();
        }

        void process_sql(const std::string& sql_content) {
            clear();

            auto blocks = preprocessor.process(sql_content);

            for (const auto& block : blocks) {
                auto token_result = lexer_streamer.tokenize(block.content);
                auto parse_result = parser_wrapper.parse(token_result.tokens.get(), block.start_line);

                errors.insert(errors.end(), parse_result.errors.begin(), parse_result.errors.end());

                if (parse_result.ast) {
                    PlSqlVisitor visitor(table_manager, procedure_manager, block.start_line);
                    visitor.visit(parse_result.ast);

                    const auto& block_violations = visitor.get_violations();
                    violations.insert(violations.end(), block_violations.begin(), block_violations.end());
                }
            }
        }
    };

    Analyzer::Analyzer() : m_impl(std::make_unique<Impl>()) {}
    Analyzer::~Analyzer() = default;

    Analyzer::Analyzer(Analyzer&&) noexcept = default;
    Analyzer& Analyzer::operator=(Analyzer&&) noexcept = default;

    bool Analyzer::analyze_string(const std::string& sql_content) {
        m_impl->process_sql(sql_content);
        return true;
    }

    bool Analyzer::analyze_file(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            m_impl->errors.push_back(ParseError{
                SourceLocation{0, 0, 0},
                "Failed to open file: " + filepath
            });
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return analyze_string(buffer.str());
    }

    std::string Analyzer::render_report(const IRenderer& renderer) const {
        return renderer.render(m_impl->table_manager, m_impl->procedure_manager, m_impl->errors, m_impl->violations);
    }

    const TableManager& Analyzer::get_table_manager() const noexcept {
        return m_impl->table_manager;
    }

    const ProcedureManager& Analyzer::get_procedure_manager() const noexcept {
        return m_impl->procedure_manager;
    }

    const std::vector<ParseError>& Analyzer::get_errors() const noexcept {
        return m_impl->errors;
    }

    const std::vector<LintViolation>& Analyzer::get_violations() const noexcept {
        return m_impl->violations;
    }

    bool Analyzer::has_critical_violations() const noexcept {
        return std::any_of(m_impl->violations.begin(), m_impl->violations.end(), [](const LintViolation& v) {
            return v.severity == LintSeverity::Critical;
        });
    }

} // namespace sqlweaver
# SQLWeaver

High-performance static analyzer, data lineage tracker, and security linter for *Oracle PL/SQL* written in *Modern C++20* and *ANTLR4*. **Operates offline without database connections.**

## 1. Core Architecture

**SQLWeaver** implements a unidirectional pipeline where each stage has a single responsibility:

| Pipeline Stage | Component | Input | Output | Responsibility |
| :--- | :--- | :--- | :--- | :--- |
| **Stage 1: Ingestion** | `Preprocessor` | Raw SQL text | `std::vector<SqlBlock>` | Normalizes CRLF to LF, splits code blocks on `/`, and indexes line offsets. |
| **Stage 2: Lexing** | `LexerStreamer` | Block text | `CommonTokenStream` | Performs case-insensitive tokenization without mutating source token text. |
| **Stage 3: Parsing** | `ParserWrapper` | `CommonTokenStream` | `ParseTree*` (AST) | Constructs the AST and captures syntax errors without aborting via panic-mode recovery. |
| **Stage 4: Traversal**| `PlSqlVisitor` | `ParseTree*` | Domain events | Extracts table/routine references, tracks I/O context (Read/Write), and flags lint rules. |
| **Stage 5: State** | `TableManager`<br>`ProcedureManager` | Domain events | Aggregated state | Deduplicates names (case-insensitive), merges I/O flags, and collects occurrences. |
| **Stage 6: Output** | `Analyzer` (Facade)<br>`IRenderer` | Aggregated state | Markdown / JSON / CSV | Formats metadata and exits with return code `2` if critical security rules are violated. |

## 2. CI/CD Gate & Linter Rules

SQLWeaver inspects code patterns to enforce production deployment policies:

- **`SEC-001` (Critical):** `UPDATE` statement without a `WHERE` clause.
- **`SEC-002` (Critical):** `DELETE` statement without a `WHERE` clause.
- **`PERF-001` (Warning):** Usage of `SELECT *` in production code.

**CI/CD Exit Codes:**
- `0`: Analysis successful, no critical violations.
- `1`: File access or parsing failure.
- `2`: Critical safety rule violated (blocks pipeline merge).

## 3. Quickstart & Build Instructions

### Prerequisites
- C++20 compiler
- CMake 3.20+
- Java Runtime Environment (JRE) for ANTLR grammar compilation

### Build Commands
```bash
# Configure and download dependencies (ANTLR runtime, Catch2)
cmake -B build

# Build engine, CLI, and test suite
cmake --build build -j $(nproc)

# Run full automated test suite
./build/sqlweaver_tests -s
```

### CLI Execution
```bash
# Markdown report (Standard)
./build/sqlweaver analyze input.sql -f md -o report.md

# JSON output (CI/CD / Data Catalogs)
./build/sqlweaver analyze input.sql -f json -o report.json

# CSV output (Spreadsheets / Audit)
./build/sqlweaver analyze input.sql -f csv -o report.csv
```

## 4. Contributing

**Contributions are welcome!** Whether it is extending the dialect coverage, writing new linting rules, or improving performance, your help makes **SQLWeaver** better for everyone.

### Priority Areas for Contribution
- **New Linter Rules:** Implementing checks for common anti-patterns (e.g., cursor leaks, dynamic SQL sanitization, missing indexes in queries).
- **Dialect Expansion:** Extending parser visitors to support additional dialects (e.g., PostgreSQL, Snowflake, T-SQL).
- **Exporters:** Adding integrations for OpenLineage, DataHub, or Mermaid.js diagram generation.
- **Corpus & Grammar Edge Cases:** Submitting tricky PL/SQL scripts to test and expand `non_reserved_keywords`.

### Development Workflow
1. **Fork & Branch:** Create a dedicated branch from `main` (e.g., `feat/rule-cursor-leak`).
2. **Follow Code Conventions:** Write clean, modern C++20 with zero external dependencies beyond ANTLR and Catch2. Keep all internal code and comments in English.
3. **Add Tests:** Every bug fix or feature must include a corresponding Catch2 test case under `tests/`.
4. **Ensure All Tests Pass:**
   ```bash
   cmake --build build && ./build/sqlweaver_tests -s
   ```
5. **Open a Pull Request:** Provide a clear description of the issue or feature along with sample SQL snippets verifying the behavior.

## 5. Appendix: Fundamental Concepts

| Concept | Definition | References |
| :--- | :--- | :--- |
| **Abstract Syntax Tree (AST)** | A hierarchical tree structure representing the syntactic elements of source code, ignoring formatting details. | [Wikipedia: Abstract Syntax Tree](https://en.wikipedia.org/wiki/Abstract_syntax_tree) |
| **Pimpl Idiom (Pointer to Implementation)** | A C++ design pattern that hides implementation details and external dependencies behind an opaque pointer to ensure stable ABI and reduce compile times. | [cppreference: Pimpl](https://en.cppreference.com/w/cpp/language/pimpl) |
| **Visitor Pattern** | A design pattern that separates algorithms from the object structures they operate on, allowing new AST traversals without changing node classes. | [Refactoring Guru: Visitor Pattern](https://refactoring.guru/design-patterns/visitor) |
| **Data Lineage** | The lifecycle tracking of data: origin, movements across tables/procedures, transformations, and consumption endpoints. | [Databricks: What is Data Lineage?](https://www.databricks.com/glossary/data-lineage) |
| **Semantic Predicates** | ANTLR4 mechanisms evaluated at runtime to dynamically disable or enable specific parser rules based on context. | [ANTLR4 Documentation: Predicates](https://github.com/antlr/antlr4/blob/master/doc/predicates.md) |
| **Soft Keywords (Non-Reserved)** | Lexical tokens that act as reserved keywords only inside specific clauses, while remaining ordinary identifiers elsewhere. | [PostgreSQL Docs: SQL Key Words](https://www.postgresql.org/docs/current/sql-keywords-appendix.html) |
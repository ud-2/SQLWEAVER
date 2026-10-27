#include "analyzer/Analyzer.hpp"
#include "renderer/MarkdownRenderer.hpp"
#include "renderer/JsonRenderer.hpp"
#include "renderer/CsvRenderer.hpp"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>

void print_usage(const char* program_name) {
    std::cout << "SQLWeaver - Oracle PL/SQL Static Analyzer & Linter\n";
    std::cout << "Usage: " << program_name << " analyze <file.sql> [-f md|json|csv] [-o output_file]\n";
}

int main(int argc, char* argv[]) {
    // Parse command-line arguments
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    std::string command = argv[1];
    if (command != "analyze") {
        std::cerr << "Unknown command: " << command << "\n";
        print_usage(argv[0]);
        return 1;
    }

    // Extract input file and optional output file and format
    std::string input_file = argv[2];
    std::string output_file = "";
    std::string format = "md";

    for (int i = 3; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-o" && i + 1 < argc) {
            output_file = argv[++i];
        } else if (arg == "-f" && i + 1 < argc) {
            format = argv[++i];
        }
    }

    // Perform analysis
    sqlweaver::Analyzer analyzer;
    std::cout << "[SQLWeaver] Analyzing file: " << input_file << " ...\n";

    if (!analyzer.analyze_file(input_file)) {
        std::cerr << "[SQLWeaver] Error: Could not open file: " << input_file << "\n";
        return 1;
    }

    // Render the report using the specified format
    std::unique_ptr<sqlweaver::IRenderer> renderer;
    if (format == "json") {
        renderer = std::make_unique<sqlweaver::JsonRenderer>();
    } else if (format == "csv") {
        renderer = std::make_unique<sqlweaver::CsvRenderer>();
    } else {
        renderer = std::make_unique<sqlweaver::MarkdownRenderer>();
    }

    // Generate the report
    std::string report = analyzer.render_report(*renderer);

    if (!output_file.empty()) {
        std::filesystem::path output_path = std::filesystem::path("output") / output_file;
        std::filesystem::create_directories(output_path.parent_path());
        std::ofstream out(output_path);

        if (out.is_open()) {
            out << report;
            std::cout << "[SQLWeaver] Report successfully written to: " << output_path << "\n";
        } else {
            std::cerr << "[SQLWeaver] Error: Could not write to output file: " << output_path << "\n";
            return 1;
        }
    } else {
        std::cout << "\n" << report << "\n";
    }

    // CI/CD Gate: Return exit code 2 if critical safety violations exist (e.g. DELETE without WHERE)
    // cf. README.md, section 2
    if (analyzer.has_critical_violations()) {
        std::cerr << "\n[SQLWeaver CI/CD Gate] CRITICAL violations found! Build failed.\n";
        return 2;
    }

    return 0;
}
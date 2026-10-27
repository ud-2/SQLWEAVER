# CMAKE CONFIGURATION
cmake_minimum_required(VERSION 3.20)
project(sqlweaver VERSION 1.0.0 LANGUAGES CXX)

set(FETCHCONTENT_QUIET FALSE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable generation of compile_commands.json for IDEs and tools (align on C++20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# FETCH DEPENDENCIES (Catch2 & ANTLR4)

# Catch2
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
)

# ANTLR4
find_package(Java REQUIRED)
set(ANTLR_VERSION "4.13.1")
set(ANTLR_JAR "${CMAKE_BINARY_DIR}/antlr-${ANTLR_VERSION}-complete.jar")

if(NOT EXISTS "${ANTLR_JAR}")
    message(STATUS "Downloading ANTLR tool...")
    file(DOWNLOAD "https://www.antlr.org/download/antlr-${ANTLR_VERSION}-complete.jar" "${ANTLR_JAR}" SHOW_PROGRESS)
endif()

FetchContent_Declare(
    antlr4_runtime
    GIT_REPOSITORY https://github.com/antlr/antlr4.git
    GIT_TAG        ${ANTLR_VERSION}
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  runtime/Cpp
)
set(ANTLR_BUILD_CPP_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(Catch2 antlr4_runtime)

# GRAMMAR PATCHING (Java -> C++ Syntax)
set(GRAMMAR_DIR "${CMAKE_SOURCE_DIR}/grammars")
set(PATCHED_GRAMMAR_DIR "${CMAKE_BINARY_DIR}/patched_grammars")
file(MAKE_DIRECTORY ${PATCHED_GRAMMAR_DIR})

function(patch_grammar G_FILE)
    if(EXISTS "${GRAMMAR_DIR}/${G_FILE}")
        file(READ "${GRAMMAR_DIR}/${G_FILE}" CONTENT)
        string(REPLACE "this." "this->" PATCHED_CONTENT "${CONTENT}")
        file(WRITE "${PATCHED_GRAMMAR_DIR}/${G_FILE}" "${PATCHED_CONTENT}")
    endif()
endfunction()

message(STATUS "Patching PL/SQL grammars for C++ support...")
patch_grammar("PlSqlLexer.g4")
patch_grammar("PlSqlParser.g4")

# 3. CODE GENERATION (ANTLR)
set(GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${GENERATED_DIR})

set(ANTLR_GENERATED_SRCS
    ${GENERATED_DIR}/PlSqlLexer.cpp
    ${GENERATED_DIR}/PlSqlParser.cpp
    ${GENERATED_DIR}/PlSqlParserVisitor.cpp
    ${GENERATED_DIR}/PlSqlParserBaseVisitor.cpp
)

add_custom_command(
    OUTPUT ${ANTLR_GENERATED_SRCS}
    COMMAND ${Java_JAVA_EXECUTABLE} -jar ${ANTLR_JAR}
            -Dlanguage=Cpp -visitor -no-listener
            -o ${GENERATED_DIR} 
            ${PATCHED_GRAMMAR_DIR}/PlSqlLexer.g4 
            ${PATCHED_GRAMMAR_DIR}/PlSqlParser.g4
    DEPENDS ${PATCHED_GRAMMAR_DIR}/PlSqlLexer.g4 ${PATCHED_GRAMMAR_DIR}/PlSqlParser.g4
    COMMENT "Generating C++ code from patched grammars..."
)

if(MSVC)
    set_source_files_properties(${ANTLR_GENERATED_SRCS} PROPERTIES COMPILE_FLAGS "/w")
else()
    set_source_files_properties(${ANTLR_GENERATED_SRCS} PROPERTIES COMPILE_FLAGS "-w")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set_source_files_properties(${GENERATED_DIR}/PlSqlParser.cpp PROPERTIES COMPILE_FLAGS "-O0 -g0")
endif()

# LIBRARY TARGETS

add_library(sqlweaver_types INTERFACE)
target_include_directories(sqlweaver_types INTERFACE ${CMAKE_SOURCE_DIR}/include)

set(LIB_SOURCES
    src/preprocessor/Preprocessor.cpp
    src/lexer/LexerStreamer.cpp
    src/parser/ParserWrapper.cpp
    src/parser/PlSqlParserBase.cpp
    src/visitor/PlSqlVisitor.cpp
    src/renderer/MarkdownRenderer.cpp
    src/renderer/JsonRenderer.cpp
    src/renderer/CsvRenderer.cpp
    src/analyzer/Analyzer.cpp
    ${ANTLR_GENERATED_SRCS}
)

add_library(sqlweaver STATIC ${LIB_SOURCES})

set(LEXER_BASE "${CMAKE_SOURCE_DIR}/include/lexer/PlSqlLexerBase.h")
set(PARSER_BASE "${CMAKE_SOURCE_DIR}/include/parser/PlSqlParserBase.h")

if(MSVC)
    set_property(SOURCE ${GENERATED_DIR}/PlSqlLexer.cpp APPEND PROPERTY COMPILE_OPTIONS "/FI${LEXER_BASE}")
    set_property(SOURCE ${GENERATED_DIR}/PlSqlParser.cpp ${GENERATED_DIR}/PlSqlParserVisitor.cpp ${GENERATED_DIR}/PlSqlParserBaseVisitor.cpp APPEND PROPERTY COMPILE_OPTIONS "/FI${PARSER_BASE}")
else()
    set_property(SOURCE ${GENERATED_DIR}/PlSqlLexer.cpp APPEND PROPERTY COMPILE_OPTIONS "-include" "${LEXER_BASE}")
    set_property(SOURCE ${GENERATED_DIR}/PlSqlParser.cpp ${GENERATED_DIR}/PlSqlParserVisitor.cpp ${GENERATED_DIR}/PlSqlParserBaseVisitor.cpp APPEND PROPERTY COMPILE_OPTIONS "-include" "${PARSER_BASE}")
endif()

target_include_directories(sqlweaver PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${GENERATED_DIR}>
    $<BUILD_INTERFACE:${antlr4_runtime_SOURCE_DIR}/runtime/Cpp/runtime/src>
)

target_include_directories(sqlweaver PRIVATE
    ${CMAKE_SOURCE_DIR}/src/include
    ${CMAKE_SOURCE_DIR}/src/visitor/include
)

target_link_libraries(sqlweaver PRIVATE antlr4_shared)

# CLI EXECUTABLE

add_executable(sqlweaver_cli src/main.cpp)
set_target_properties(sqlweaver_cli PROPERTIES OUTPUT_NAME "sqlweaver")
target_link_libraries(sqlweaver_cli PRIVATE sqlweaver)

# TEST EXECUTABLE

enable_testing()

add_executable(sqlweaver_tests
    tests/test_domain_model.cpp
    tests/test_preprocessor.cpp
    tests/test_lexer.cpp
    tests/test_parser.cpp
    tests/test_visitor.cpp
    tests/test_renderer.cpp
    tests/test_e2e.cpp
)

target_link_libraries(sqlweaver_tests
    PRIVATE
        sqlweaver
        sqlweaver_types
        Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(sqlweaver_tests)
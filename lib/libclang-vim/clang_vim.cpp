#include <unistd.h>
#include <tuple>

#include <clang-c/Index.h>
#include <clang-c/CXCompilationDatabase.h>

#include "helpers.hpp"
#include "tokenizer.hpp"
#include "AST_extracter.hpp"
#include "location.hpp"
#include "deduction.hpp"

/// Ensures that writes to stderr are ignored.
class stderr_guard
{
    int m_stderr;

public:
    stderr_guard()
        : m_stderr(dup(STDERR_FILENO))
    {
        // Close stderr.
        close(STDERR_FILENO);
    }

    ~stderr_guard()
    {
        // Restore stderr.
        dup(m_stderr);
        close(m_stderr);
    }
};

namespace libclang_vim
{

/// Look up compilation arguments for a file from a database in one of its parent directories.
args_type parse_compilation_database(const std::string& file)
{
    args_type ret;

    std::size_t found = file.find_last_of("/\\");
    std::string directory = file.substr(0, found);
    while (true)
    {
        std::string json = directory + file.substr(found, 1) + "compile_commands.json";
        std::ifstream stream(json.c_str());
        if (stream.good())
            break;

        found = directory.find_last_of("/\\");
        if (found == std::string::npos)
            break;

        directory = directory.substr(0, found);
    }

    if (directory.empty())
    {
        // Our default when no JSON was found.
        ret.push_back("-std=c++1y");

        return ret;
    }

    CXCompilationDatabase_Error error;
    CXCompilationDatabase database = clang_CompilationDatabase_fromDirectory(directory.c_str(), &error);
    if (error == CXCompilationDatabase_NoError)
    {
        CXCompileCommands commands = clang_CompilationDatabase_getCompileCommands(database, file.c_str());
        unsigned commandsSize = clang_CompileCommands_getSize(commands);
        if (commandsSize >= 1)
        {
            CXCompileCommand command = clang_CompileCommands_getCommand(commands, 0);
            unsigned args = clang_CompileCommand_getNumArgs(command);
            for (unsigned i = 0; i < args; ++i)
            {
                cxstring_ptr arg = clang_CompileCommand_getArg(command, i);
                if (file != clang_getCString(arg))
                    ret.push_back(clang_getCString(arg));
            }
        }
        clang_CompileCommands_dispose(commands);
    }
    clang_CompilationDatabase_dispose(database);

    return ret;
}

char const* get_compile_commands(const std::string& file)
{
    static std::string vimson;

    // Write the header.
    std::stringstream ss;
    ss << "{'commands':'";

    args_type args = parse_compilation_database(file);
    for (std::size_t i = 0; i < args.size(); ++i)
    {
        if (i)
            ss << " ";
        ss << args[i];
    }

    // Write the footer.
    ss << "'}";
    vimson = ss.str();
    return vimson.c_str();
}

char const* get_completion_at(location_tuple const& location_info)
{
    static std::string vimson;

    // Write the header.
    std::stringstream ss;
    ss << "['";

    // Write the completion list.
    libclang_vim::cxindex_ptr index = clang_createIndex(/*excludeDeclsFromPCH=*/1, /*displayDiagnostics=*/0);

    std::string file_name = location_info.file;
    std::vector<const char*> args_ptrs = get_args_ptrs(location_info.args);
    cxtranslation_unit_ptr translation_unit(clang_parseTranslationUnit(index, file_name.c_str(), args_ptrs.data(), args_ptrs.size(), nullptr, 0, CXTranslationUnit_Incomplete));
    if (!translation_unit)
        return "[]";

    unsigned line = location_info.line;
    unsigned column = location_info.col;
    CXCodeCompleteResults* results = clang_codeCompleteAt(translation_unit, file_name.c_str(), line, column, nullptr, 0, clang_defaultCodeCompleteOptions());
    std::set<std::string> matches;
    if (results)
    {
        for (unsigned i = 0; i < results->NumResults; ++i)
        {
            const CXCompletionString& completion_string = results->Results[i].CompletionString;
            std::stringstream match;
            for (unsigned j = 0; j < clang_getNumCompletionChunks(completion_string); ++j)
            {
                if (clang_getCompletionChunkKind(completion_string, j) != CXCompletionChunk_TypedText)
                    continue;

                const CXString& chunk_text = clang_getCompletionChunkText(completion_string, j);
                match << clang_getCString(chunk_text);
            }
            matches.insert(match.str());
        }
        clang_disposeCodeCompleteResults(results);
    }
    for (auto it = matches.begin(); it != matches.end(); ++it)
    {
        if (it != matches.begin())
            ss << "', '";
        ss << *it;
    }

    // Write the footer.
    ss << "']";
    vimson = ss.str();
    return vimson.c_str();
}

} // namespace libclang_vim

extern "C" {

char const* vim_clang_version()
{
    return clang_getCString(clang_getClangVersion());
}

char const* vim_clang_tokens(char const* arguments)
{
    auto const parsed = libclang_vim::parse_default_args(arguments);
    auto const args_ptrs = libclang_vim::get_args_ptrs(parsed.second);
    libclang_vim::tokenizer tokenizer{parsed.first};
    static auto const vimson = tokenizer.tokenize_as_vimson(args_ptrs.data(), args_ptrs.size());
    return vimson.c_str();
}

// API to extract AST nodes {{{
// API to extract all {{{
char const* vim_clang_extract_all(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::all,
                [](CXCursor const&){ return true; }
            );
}

char const* vim_clang_extract_declarations(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::all,
                [](CXCursor const& c){ return clang_isDeclaration(clang_getCursorKind(c)); }
            );
}

char const* vim_clang_extract_attributes(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::all,
                [](CXCursor const& c){ return clang_isAttribute(clang_getCursorKind(c)); }
            );
}

char const* vim_clang_extract_expressions(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::all,
                [](CXCursor const& c){ return clang_isExpression(clang_getCursorKind(c)); }
            );
}

char const* vim_clang_extract_preprocessings(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::all,
                [](CXCursor const& c){ return clang_isPreprocessing(clang_getCursorKind(c)); }
            );
}

char const* vim_clang_extract_references(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::all,
                [](CXCursor const& c){ return clang_isReference(clang_getCursorKind(c)); }
            );
}

char const* vim_clang_extract_statements(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::all,
                [](CXCursor const& c){ return clang_isStatement(clang_getCursorKind(c)); }
            );
}

char const* vim_clang_extract_translation_units(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::all,
                [](CXCursor const& c){ return clang_isTranslationUnit(clang_getCursorKind(c)); }
            );
}

char const* vim_clang_extract_definitions(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::all,
                [](CXCursor const& c){ return clang_isCursorDefinition(c); }
            );
}

char const* vim_clang_extract_virtual_member_functions(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::all,
                [](CXCursor const& c){ return clang_CXXMethod_isVirtual(c); }
            );
}

char const* vim_clang_extract_pure_virtual_member_functions(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::all,
                [](CXCursor const& c){ return clang_CXXMethod_isPureVirtual(c); }
            );
}

char const* vim_clang_extract_static_member_functions(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::all,
                [](CXCursor const& c){ return clang_CXXMethod_isStatic(c); }
            );
}
// }}}

// API to extract current file only {{{
char const* vim_clang_extract_all_current_file(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::current_file,
                [](CXCursor const&) -> bool {
                    return true;
                }
            );
}

char const* vim_clang_extract_declarations_current_file(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::current_file,
                [](CXCursor const& c){
                    return clang_isDeclaration(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_attributes_current_file(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::current_file,
                [](CXCursor const& c){
                    return clang_isAttribute(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_expressions_current_file(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::current_file,
                [](CXCursor const& c){
                    return clang_isExpression(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_preprocessings_current_file(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::current_file,
                [](CXCursor const& c){
                    return clang_isPreprocessing(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_references_current_file(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::current_file,
                [](CXCursor const& c){
                    return clang_isReference(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_statements_current_file(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::current_file,
                [](CXCursor const& c){
                    return clang_isStatement(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_translation_units_current_file(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::current_file,
                [](CXCursor const& c){
                    return clang_isTranslationUnit(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_definitions_current_file(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::current_file,
                clang_isCursorDefinition
            );
}

char const* vim_clang_extract_virtual_member_functions_current_file(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::current_file,
                clang_CXXMethod_isVirtual
            );
}

char const* vim_clang_extract_pure_virtual_member_functions_current_file(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::current_file,
                clang_CXXMethod_isPureVirtual
            );
}

char const* vim_clang_extract_static_member_functions_current_file(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::current_file,
                clang_CXXMethod_isStatic
            );
}
// }}}

// API to extract current file only {{{
char const* vim_clang_extract_all_non_system_headers(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::non_system_headers,
                [](CXCursor const&) -> bool {
                    return true;
                }
            );
}

char const* vim_clang_extract_declarations_non_system_headers(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::non_system_headers,
                [](CXCursor const& c){
                    return clang_isDeclaration(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_attributes_non_system_headers(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::non_system_headers,
                [](CXCursor const& c){
                    return clang_isAttribute(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_expressions_non_system_headers(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::non_system_headers,
                [](CXCursor const& c){
                    return clang_isExpression(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_preprocessings_non_system_headers(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::non_system_headers,
                [](CXCursor const& c){
                    return clang_isPreprocessing(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_references_non_system_headers(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::non_system_headers,
                [](CXCursor const& c){
                    return clang_isReference(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_statements_non_system_headers(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::non_system_headers,
                [](CXCursor const& c){
                    return clang_isStatement(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_translation_units_non_system_headers(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::non_system_headers,
                [](CXCursor const& c){
                    return clang_isTranslationUnit(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_definitions_non_system_headers(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::non_system_headers,
                clang_isCursorDefinition
            );
}

char const* vim_clang_extract_virtual_member_functions_non_system_headers(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::non_system_headers,
                clang_CXXMethod_isVirtual
            );
}

char const* vim_clang_extract_pure_virtual_member_functions_non_system_headers(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::non_system_headers,
                clang_CXXMethod_isPureVirtual
            );
}

char const* vim_clang_extract_static_member_functions_non_system_headers(char const* arguments)
{
    return libclang_vim::extract_AST_nodes(
                arguments,
                libclang_vim::extraction_policy::non_system_headers,
                clang_CXXMethod_isStatic
            );
}
// }}}
// }}}

// API to get information of specific location {{{
char const* vim_clang_get_location_information(char const* location_string)
{
    auto const location_info = libclang_vim::parse_args_with_location(location_string);
    char const* file_name = location_info.file.c_str();
    auto const args_ptrs = libclang_vim::get_args_ptrs(location_info.args);
    libclang_vim::cxindex_ptr index = clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/0);
    libclang_vim::cxtranslation_unit_ptr translation_unit(clang_parseTranslationUnit(index, file_name, args_ptrs.data(), args_ptrs.size(), nullptr, 0, CXTranslationUnit_Incomplete));
    if (!translation_unit)
        return "{}";

    CXFile const file = clang_getFile(translation_unit, file_name);
    auto const location = clang_getLocation(translation_unit, file, location_info.line, location_info.col);
    CXCursor const cursor = clang_getCursor(translation_unit, location);
    static std::string result;
    result = "{" + libclang_vim::stringize_cursor(cursor, clang_getCursorSemanticParent(cursor)) + "}";

    return result.c_str();
}
// }}}

// API to get extent of identifier at specific location {{{
char const* vim_clang_get_extent_of_node_at_specific_location(char const* location_string)
{
    auto location_info = libclang_vim::parse_args_with_location(location_string);
    char const* file_name = location_info.file.c_str();
    auto const args_ptrs = libclang_vim::get_args_ptrs(location_info.args);
    libclang_vim::cxindex_ptr index = clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/0);
    libclang_vim::cxtranslation_unit_ptr translation_unit(clang_parseTranslationUnit(index, file_name, args_ptrs.data(), args_ptrs.size(), nullptr, 0, CXTranslationUnit_Incomplete));
    if (!translation_unit)
        return "{}";

    CXFile const file = clang_getFile(translation_unit, file_name);
    auto const location = clang_getLocation(translation_unit, file, location_info.line, location_info.col);
    CXCursor const cursor = clang_getCursor(translation_unit, location);
    static std::string result;
    result = "{" + libclang_vim::stringize_extent(cursor) + "}";

    return result.c_str();
}

char const* vim_clang_get_inner_definition_extent_at_specific_location(char const* location_string)
{
    auto const parsed_location = libclang_vim::parse_args_with_location(location_string);
    return libclang_vim::get_extent(
                parsed_location,
                clang_isCursorDefinition
            );
}

char const* vim_clang_get_expression_extent_at_specific_location(char const* location_string)
{
    auto const parsed_location = libclang_vim::parse_args_with_location(location_string);
    return libclang_vim::get_extent(
                parsed_location,
                [](CXCursor const& c){
                    return clang_isExpression(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_get_statement_extent_at_specific_location(char const* location_string)
{
    auto const parsed_location = libclang_vim::parse_args_with_location(location_string);
    return libclang_vim::get_extent(
                parsed_location,
                [](CXCursor const& c){
                    return clang_isStatement(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_get_class_extent_at_specific_location(char const* location_string)
{
    auto const parsed_location = libclang_vim::parse_args_with_location(location_string);
    return libclang_vim::get_extent(
                parsed_location,
                libclang_vim::is_class_decl
            );
}

char const* vim_clang_get_function_extent_at_specific_location(char const* location_string)
{
    auto const parsed_location = libclang_vim::parse_args_with_location(location_string);
    return libclang_vim::get_extent(
                parsed_location,
                libclang_vim::is_function_decl
            );
}

char const* vim_clang_get_parameter_extent_at_specific_location(char const* location_string)
{
    auto const parsed_location = libclang_vim::parse_args_with_location(location_string);
    return libclang_vim::get_extent(
                parsed_location,
                libclang_vim::is_parameter
            );
}

char const* vim_clang_get_namespace_extent_at_specific_location(char const* location_string)
{
    auto const parsed_location = libclang_vim::parse_args_with_location(location_string);
    return libclang_vim::get_extent(
                parsed_location,
                [](CXCursor const& c){
                    return clang_getCursorKind(c) == CXCursor_Namespace;
                }
            );
}
// }}}

char const* vim_clang_get_definition_at(char const* location_string)
{
    auto const parsed_location = libclang_vim::parse_args_with_location(location_string);
    return libclang_vim::get_related_node_of(
            parsed_location,
            clang_getCursorDefinition
        );
}

char const* vim_clang_get_referenced_at(char const* location_string)
{
    auto const parsed_location = libclang_vim::parse_args_with_location(location_string);
    return libclang_vim::get_related_node_of(
            parsed_location,
            clang_getCursorReferenced
        );
}

char const* vim_clang_get_declaration_at(char const* location_string)
{
    stderr_guard g;
    auto const parsed_location = libclang_vim::parse_args_with_location(location_string);
    return libclang_vim::get_related_node_of(
            parsed_location,
            clang_getCanonicalCursor
        );
}

char const* vim_clang_get_pointee_type_at(char const* location_string)
{
    auto const parsed_location = libclang_vim::parse_args_with_location(location_string);
    return libclang_vim::get_type_related_to(
            parsed_location,
            clang_getPointeeType
        );
}

char const* vim_clang_get_canonical_type_at(char const* location_string)
{
    auto const parsed_location = libclang_vim::parse_args_with_location(location_string);
    return libclang_vim::get_type_related_to(
            parsed_location,
            clang_getCanonicalType
        );
}

char const* vim_clang_get_result_type_at(char const* location_string)
{
    auto const parsed_location = libclang_vim::parse_args_with_location(location_string);
    return libclang_vim::get_type_related_to(
            parsed_location,
            clang_getResultType
        );
}

char const* vim_clang_get_class_type_of_member_pointer_at(char const* location_string)
{
    auto const parsed_location = libclang_vim::parse_args_with_location(location_string);
    return libclang_vim::get_type_related_to(
            parsed_location,
            clang_Type_getClassType
        );
}

char const* vim_clang_get_all_extents_at(char const* location_string)
{
    return libclang_vim::get_all_extents(libclang_vim::parse_args_with_location(location_string));
}

char const* vim_clang_deduce_var_decl_at(char const* location_string)
{
    return libclang_vim::deduce_var_decl_type(libclang_vim::parse_args_with_location(location_string));
}

char const* vim_clang_deduce_func_decl_at(char const* location_string)
{
    return libclang_vim::deduce_func_return_type(libclang_vim::parse_args_with_location(location_string));
}

char const* vim_clang_deduce_func_or_var_decl_at(char const* location_string)
{
    return libclang_vim::deduce_func_or_var_decl(libclang_vim::parse_args_with_location(location_string));
}

char const* vim_clang_get_type_with_deduction_at(char const* location_string)
{
    stderr_guard g;

    const char* ret = libclang_vim::deduce_type_at(libclang_vim::parse_args_with_location(location_string));
    return ret;
}

char const* vim_clang_get_current_function_at(char const* location_string)
{
    stderr_guard g;

    const char* ret = libclang_vim::get_current_function_at(libclang_vim::parse_args_with_location(location_string));
    return ret;
}

char const* vim_clang_get_completion_at(char const* location_string)
{
    stderr_guard g;

    const char* ret = libclang_vim::get_completion_at(libclang_vim::parse_args_with_location(location_string));
    return ret;
}

char const* vim_clang_get_comment_at(char const* location_string)
{
    stderr_guard g;

    const char* ret = libclang_vim::get_comment_at(libclang_vim::parse_args_with_location(location_string));
    return ret;
}

char const* vim_clang_get_deduced_declaration_at(char const* location_string)
{
    stderr_guard g;

    const char* ret = libclang_vim::get_deduced_declaration_at(libclang_vim::parse_args_with_location(location_string));
    return ret;
}

char const* vim_clang_get_include_at(const char* location_string)
{
    stderr_guard g;

    const char* ret = libclang_vim::get_include_at(libclang_vim::parse_args_with_location(location_string));
    return ret;
}

char const* vim_clang_get_compile_commands(char const* file)
{
    stderr_guard g;

    const char* ret = libclang_vim::get_compile_commands(libclang_vim::parse_default_args(file).first);
    return ret;
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

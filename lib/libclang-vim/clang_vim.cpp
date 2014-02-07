#include <tuple>

#include <clang-c/Index.h>

#include "helpers.hpp"
#include "tokenizer.hpp"
#include "AST_extracter.hpp"
#include "location.hpp"
#include "deduction.hpp"

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
    char const* file_name = std::get<0>(location_info).c_str();
    auto const args_ptrs = libclang_vim::get_args_ptrs(std::get<1>(location_info));
    CXIndex index = clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/0);
    CXTranslationUnit translation_unit = clang_parseTranslationUnit(index, file_name, args_ptrs.data(), args_ptrs.size(), NULL, 0, CXTranslationUnit_Incomplete);
    if (translation_unit == NULL) {
        clang_disposeIndex(index);
        return "{}";
    }

    CXFile const file = clang_getFile(translation_unit, file_name);
    auto const location = clang_getLocation(translation_unit, file, std::get<2>(location_info), std::get<3>(location_info));
    CXCursor const cursor = clang_getCursor(translation_unit, location);
    static std::string result;
    result = "{" + libclang_vim::stringize_cursor(cursor, clang_getCursorSemanticParent(cursor)) + "}";

    clang_disposeTranslationUnit(translation_unit);
    clang_disposeIndex(index);

    return result.c_str();
}
// }}}

// API to get extent of identifier at specific location {{{
char const* vim_clang_get_extent_of_node_at_specific_location(char const* location_string)
{
    auto location_info = libclang_vim::parse_args_with_location(location_string);
    char const* file_name = std::get<0>(location_info).c_str();
    auto const args_ptrs = libclang_vim::get_args_ptrs(std::get<1>(location_info));
    CXIndex index = clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/0);
    CXTranslationUnit translation_unit = clang_parseTranslationUnit(index, file_name, args_ptrs.data(), args_ptrs.size(), NULL, 0, CXTranslationUnit_Incomplete);
    if (translation_unit == NULL) {
        clang_disposeIndex(index);
        return "{}";
    }

    CXFile const file = clang_getFile(translation_unit, file_name);
    auto const location = clang_getLocation(translation_unit, file, std::get<2>(location_info), std::get<3>(location_info));
    CXCursor const cursor = clang_getCursor(translation_unit, location);
    static std::string result;
    result = "{" + libclang_vim::stringize_extent(cursor) + "}";

    clang_disposeTranslationUnit(translation_unit);
    clang_disposeIndex(index);

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
    return libclang_vim::deduce_type_at(libclang_vim::parse_args_with_location(location_string));
}

} // extern "C"


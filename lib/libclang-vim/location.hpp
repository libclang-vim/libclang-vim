#if !defined LIBCLANG_VIM_LOCATION_HPP_INCLUDED
#define      LIBCLANG_VIM_LOCATION_HPP_INCLUDED

#include <tuple>
#include <string>
#include <cstdio>

#include <clang-c/Index.h>

#include "helpers.hpp"
#include "stringizers.hpp"

namespace libclang_vim {

namespace detail {

    CXCursor search_AST_upward(CXCursor cursor, const std::function<unsigned(CXCursor)>& predicate)
    {
        while (!clang_isInvalid(clang_getCursorKind(cursor))) {
            if (predicate(cursor)){
                return cursor;
            }
            cursor = clang_getCursorSemanticParent(cursor);
        }
        return clang_getNullCursor();
    }

} // namespace detail

char const* get_extent(const location_tuple& location_info, const std::function<unsigned(CXCursor)>& predicate)
{
    return at_specific_location(
                location_info,
                [&predicate](CXCursor const& c) -> std::string {
                    CXCursor const rc = detail::search_AST_upward(c, predicate);
                    if (clang_Cursor_isNull(rc)) {
                        return "{}";
                    } else {
                        return "{" + stringize_extent(rc) + "}";
                    }
                });
};

const char* get_related_node_of(const location_tuple& location_info, const std::function<CXCursor(CXCursor)>& predicate)
{
    return at_specific_location(
                location_info,
                [&predicate](CXCursor const& c) -> std::string {
                    CXCursor const rc = predicate(c);
                    if (clang_isInvalid(clang_getCursorKind(rc))) {
                        return "{}";
                    } else {
                        return "{" + stringize_cursor(rc, clang_getCursorSemanticParent(rc)) + "}";
                    }
                });
}

const char* get_type_related_to(const location_tuple& location_info, const std::function<CXType(CXType)>& predicate)
{
    return at_specific_location(
            location_info,
            [&predicate](CXCursor const& c) -> std::string {
                CXType const type = predicate(clang_getCursorType(c));
                if (type.kind == CXType_Invalid) {
                    return "{}";
                } else {
                    return "{" + stringize_type(type)+ "}";
                }
            });
}

inline const char* get_all_extents(const location_tuple& location_info)
{
    static std::string vimson;
    vimson = "";
    char const* file_name = location_info.file.c_str();

    cxindex_ptr index = clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/0);
    auto const args_ptrs = get_args_ptrs(location_info.args);
    cxtranslation_unit_ptr translation_unit(clang_parseTranslationUnit(index, file_name, args_ptrs.data(), args_ptrs.size(), nullptr, 0, CXTranslationUnit_Incomplete));
    if (!translation_unit)
        return "[]";

    CXFile const file = clang_getFile(translation_unit, file_name);
    auto const location = clang_getLocation(translation_unit, file, location_info.line, location_info.col);
    CXCursor cursor = clang_getCursor(translation_unit, location);
    vimson += "{" + stringize_extent(cursor) + "},";

    bool already_pass_expression = false, already_pass_statement = false;
    while (!clang_isInvalid(clang_getCursorKind(cursor))) {
        if ( is_class_decl(cursor) ||
             is_function_decl(cursor) ||
             clang_getCursorKind(cursor) == CXCursor_Namespace ||
             (!already_pass_expression && clang_isExpression(clang_getCursorKind(cursor))) ||
             (!already_pass_statement && clang_isStatement(clang_getCursorKind(cursor)))
           ) {
            vimson += "{" + stringize_extent(cursor) + "},";
        }
        cursor = clang_getCursorSemanticParent(cursor);
    }

    vimson = "[" + vimson + "]";

    return vimson.c_str();
}

} // namespace libclang_vim

#endif    // LIBCLANG_VIM_LOCATION_HPP_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

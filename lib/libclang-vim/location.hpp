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

    template<class Predicate>
    CXCursor search_AST_upward(CXCursor cursor, Predicate const& predicate)
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

template<class LocationTuple, class Predicate>
auto get_extent(
        LocationTuple const& location_tuple,
        Predicate const& predicate
    ) -> char const*
{
    return at_specific_location(
                location_tuple,
                [&predicate](CXCursor const& c) -> std::string {
                    CXCursor const rc = detail::search_AST_upward(c, predicate);
                    if (clang_Cursor_isNull(rc)) {
                        return "{}";
                    } else {
                        return "{" + stringize_extent(rc) + "}";
                    }
                });
};

template<class LocationTuple, class JumpFunc>
inline auto get_related_node_of(
        LocationTuple const& location_tuple,
        JumpFunc const& predicate
    ) -> char const*
{
    return at_specific_location(
                location_tuple,
                [&predicate](CXCursor const& c) -> std::string {
                    CXCursor const rc = predicate(c);
                    if (clang_isInvalid(clang_getCursorKind(rc))) {
                        return "{}";
                    } else {
                        return "{" + stringize_cursor(rc, clang_getCursorSemanticParent(rc)) + "}";
                    }
                });
}

template<class LocationTuple, class JumpFunc>
inline auto get_type_related_to(
        LocationTuple const& location_tuple,
        JumpFunc const& predicate
    ) -> char const*
{
    return at_specific_location(
            location_tuple,
            [&predicate](CXCursor const& c) -> std::string {
                CXType const type = predicate(clang_getCursorType(c));
                if (type.kind == CXType_Invalid) {
                    return "{}";
                } else {
                    return "{" + stringize_type(type)+ "}";
                }
            });
}

template<class LocationTuple>
auto get_all_extents( LocationTuple const& location_tuple)
    -> char const*
{
    static std::string vimson;
    vimson = "";
    char const* file_name = std::get<0>(location_tuple).c_str();

    CXIndex index = clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/0);
    auto const args_ptrs = get_args_ptrs(std::get<1>(location_tuple));
    CXTranslationUnit translation_unit = clang_parseTranslationUnit(index, file_name, args_ptrs.data(), args_ptrs.size(), NULL, 0, CXTranslationUnit_Incomplete);
    if (translation_unit == NULL) {
        clang_disposeIndex(index);
        return "[]";
    }

    CXFile const file = clang_getFile(translation_unit, file_name);
    auto const location = clang_getLocation(translation_unit, file, std::get<2>(location_tuple), std::get<3>(location_tuple));
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

    clang_disposeTranslationUnit(translation_unit);
    clang_disposeIndex(index);

    return vimson.c_str();
}

} // namespace libclang_vim

#endif    // LIBCLANG_VIM_LOCATION_HPP_INCLUDED

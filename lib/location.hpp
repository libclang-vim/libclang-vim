#if !defined LIBCLANG_VIM_LOCATION_HPP_INCLUDED
#define      LIBCLANG_VIM_LOCATION_HPP_INCLUDED

#include <tuple>
#include <string>
#include <cstdio>
#include <algorithm>


#include <clang-c/Index.h>

#include "helpers.hpp"
#include "stringizers.hpp"

namespace libclang_vim {

// Get extent {{{
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
        Predicate const& predicate,
        char const* argv[] = {},
        int const argc = 0
    ) -> char const*
{
    static std::string vimson;
    char const* file_name = std::get<2>(location_tuple).c_str();

    CXIndex index = clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/0);
    CXTranslationUnit translation_unit = clang_parseTranslationUnit(index, file_name, argv, argc, NULL, 0, CXTranslationUnit_Incomplete);
    if (translation_unit == NULL) {
        clang_disposeIndex(index);
        return "{}";
    }

    CXFile const file = clang_getFile(translation_unit, file_name);
    auto const location = clang_getLocation(translation_unit, file, std::get<0>(location_tuple), std::get<1>(location_tuple));
    CXCursor const result_cursor = detail::search_AST_upward(
            clang_getCursor(translation_unit, location),
            predicate
        );

    if (!clang_Cursor_isNull(result_cursor)) {
        auto const range = clang_getCursorExtent(result_cursor);
        vimson = "{" + stringize_range(range) + "}";
    } else {
        vimson = "{}";
    }

    clang_disposeTranslationUnit(translation_unit);
    clang_disposeIndex(index);

    return vimson.c_str();
};
// }}}

// Get related location
template<class LocationTuple, class JumpFunc>
auto get_related_node_of(
        LocationTuple const& location_tuple,
        JumpFunc const& predicate,
        char const* argv[] = {},
        int const argc = 0
    ) -> char const*
{
    static std::string vimson;
    char const* file_name = std::get<2>(location_tuple).c_str();

    CXIndex index = clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/0);
    CXTranslationUnit translation_unit = clang_parseTranslationUnit(index, file_name, argv, argc, NULL, 0, CXTranslationUnit_Incomplete);
    if (translation_unit == NULL) {
        clang_disposeIndex(index);
        return "{}";
    }

    CXFile const file = clang_getFile(translation_unit, file_name);
    auto const location = clang_getLocation(translation_unit, file, std::get<0>(location_tuple), std::get<1>(location_tuple));

    auto const result_cursor = predicate(clang_getCursor(translation_unit, location));
    if (clang_isInvalid(clang_getCursorKind(result_cursor))) {
        vimson = "{}";
    } else {
        vimson = "{" + stringize_cursor(
                        result_cursor,
                        clang_getCursorSemanticParent(result_cursor)
                    ) + "}";
    }

    clang_disposeTranslationUnit(translation_unit);
    clang_disposeIndex(index);

    return vimson.c_str();
}

} // namespace libclang_vim

#endif    // LIBCLANG_VIM_LOCATION_HPP_INCLUDED

#if !defined LIBCLANG_VIM_DEDUCTION_HPP_INCLUDED
#define      LIBCLANG_VIM_DEDUCTION_HPP_INCLUDED

#include <string>

#include "helpers.hpp"
#include "stringizers.hpp"

namespace libclang_vim {

namespace detail {

CXType deduct_cursor_type(CXCursor const& cursor)
{
    auto const type = clang_getCursorType(cursor);
    if (type.kind == CXType_Unexposed) {
        // TODO: Should deduct
        return type;
    } else {
        return type;
    }
}

} // namespace detail

template<class LocationTuple>
inline char const* deduct_var_decl_type(LocationTuple const& location_tuple, char const* argv[] = {}, int const argc = 0)
{
    return at_specific_location(
                location_tuple,
                [](CXCursor const& cursor)
                    -> std::string
                {
                    CXCursor const var_decl_cursor = search_kind(cursor, CXCursor_VarDecl);
                    if (clang_Cursor_isNull(var_decl_cursor)) {
                        return "{}";
                    }

                    CXType const var_type = detail::deduct_cursor_type(var_decl_cursor);

                    std::string result;
                    result += stringize_type(var_type);
                    result += "'canonical':{" + stringize_type(clang_getCanonicalType(var_type)) + "},";
                    return "{" + result + "}";
                },
                argv,
                argc
            );
}

} // namespace libclang_vim

#endif    // LIBCLANG_VIM_DEDUCTION_HPP_INCLUDED

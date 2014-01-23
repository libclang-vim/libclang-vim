#if !defined LIBCLANG_VIM_DEDUCTION_HPP_INCLUDED
#define      LIBCLANG_VIM_DEDUCTION_HPP_INCLUDED

#include <string>

#include "helpers.hpp"
#include "stringizers.hpp"

namespace libclang_vim {

namespace detail {

CXChildVisitResult rhs_type_deducter(CXCursor cursor, CXCursor, CXClientData data)
{
    auto const type = clang_getCursorType(cursor);

    if (type.kind == CXType_Unexposed || type.kind == CXType_Invalid) {
        clang_visitChildren(cursor, rhs_type_deducter, data);
        return CXChildVisit_Continue;
    } else {
        *(reinterpret_cast<CXType *>(data)) = type;
        return CXChildVisit_Break;
    }
}

CXType deduct_type_at_cursor(CXCursor const& cursor)
{
    auto const type = clang_getCursorType(cursor);
    if (type.kind == CXType_Unexposed || type.kind == CXType_Invalid) {
        CXType deducted_type;
        deducted_type.kind = CXType_Invalid;
        clang_visitChildren(cursor, rhs_type_deducter, &deducted_type);
        return deducted_type;
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

                    CXType const var_type = detail::deduct_type_at_cursor(var_decl_cursor);
                    if (var_type.kind == CXType_Invalid) {
                        return "{}";
                    }

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

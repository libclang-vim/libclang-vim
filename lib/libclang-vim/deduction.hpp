#if !defined LIBCLANG_VIM_DEDUCTION_HPP_INCLUDED
#define      LIBCLANG_VIM_DEDUCTION_HPP_INCLUDED

#include <string>

#include "helpers.hpp"
#include "stringizers.hpp"

namespace libclang_vim {

namespace detail {

CXChildVisitResult unexposed_type_deducter(CXCursor cursor, CXCursor, CXClientData data)
{
    auto const type = clang_getCursorType(cursor);
    switch (type.kind) {
        case CXType_Unexposed: {
            auto const type_name = owned(clang_getTypeSpelling(type));
            if (std::strcmp(to_c_str(type_name), "auto") != 0) {
                *(reinterpret_cast<CXType *>(data)) = type;
                return CXChildVisit_Break;
            }
        }
        case CXType_Invalid: {
            // When (unexposed and "auto") or invalid
            clang_visitChildren(cursor, unexposed_type_deducter, data);
            return CXChildVisit_Continue;
        }
        default: {
            *(reinterpret_cast<CXType *>(data)) = type;
            return CXChildVisit_Break;
        }
    }
}

CXType deduct_type_at_cursor(CXCursor const& cursor)
{
    auto const type = clang_getCursorType(cursor);
    switch (type.kind) {
        case CXType_Unexposed: {
            auto const type_name = owned(clang_getTypeSpelling(type));
            if (std::strcmp(to_c_str(type_name), "auto") != 0) {
                return type;
            }
        }
        case CXType_Invalid: {
            // When (unexposed and "auto") or invalid
            CXType deducted_type;
            deducted_type.kind = CXType_Invalid;
            clang_visitChildren(cursor, unexposed_type_deducter, &deducted_type);
            return deducted_type;
        }
        default: return type;
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
                    CXCursor const var_decl_cursor = search_kind(cursor, [](CXCursorKind const& kind){ return kind == CXCursor_VarDecl; });
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

namespace detail {

CXType deduct_func_decl_type_at_cursor(CXCursor const& cursor)
{
    auto const func_type = clang_getCursorType(cursor);
    auto const result_type = clang_getResultType(func_type);

    switch (result_type.kind) {
        case CXType_Unexposed: {
            auto const type_name = owned(clang_getTypeSpelling(result_type));
            if (std::strcmp(to_c_str(type_name), "auto") != 0) {
                return result_type;
            }
        }
        case CXType_Invalid: {
            // When (unexposed and "auto") or invalid

            // Get cursor at a return statement
            CXCursor const return_stmt_cursor = search_kind(cursor, [](CXCursorKind const& kind){ return kind == CXCursor_ReturnStmt; });
            if (clang_Cursor_isNull(return_stmt_cursor)) {
                return clang_getCursorType(return_stmt_cursor);
            }

            CXType deducted_type;
            deducted_type.kind = CXType_Invalid;
            clang_visitChildren(return_stmt_cursor, unexposed_type_deducter, &deducted_type);
            return deducted_type;
        }
        default: return result_type;
    }
}

} // namespace detail

template<class LocationTuple>
inline char const* deduct_func_return_type(LocationTuple const& location_tuple, char const* argv[] = {}, int const argc = 0)
{
    return at_specific_location(
                location_tuple,
                [](CXCursor const& cursor)
                    -> std::string
                {
                    CXCursor const func_decl_cursor = search_kind(cursor, [](CXCursorKind const& kind){ return is_function_decl_kind(kind); });
                    if (clang_Cursor_isNull(func_decl_cursor)) {
                        return "{}";
                    }

                    CXType const func_type = detail::deduct_func_decl_type_at_cursor(func_decl_cursor);
                    if (func_type.kind == CXType_Invalid) {
                        return "{}";
                    }

                    std::string result;
                    result += stringize_type(func_type);
                    result += "'canonical':{" + stringize_type(clang_getCanonicalType(func_type)) + "},";
                    return "{" + result + "}";
                    return "";
                },
                argv,
                argc
            );
}

} // namespace libclang_vim

#endif    // LIBCLANG_VIM_DEDUCTION_HPP_INCLUDED

#if !defined LIBCLANG_VIM_DEDUCTION_HPP_INCLUDED
#define      LIBCLANG_VIM_DEDUCTION_HPP_INCLUDED

#include <cctype>
#include <string>

#include "helpers.hpp"
#include "stringizers.hpp"

namespace libclang_vim {

namespace detail {

bool is_auto_type(std::string const& type_name)
{
    for ( auto pos = type_name.find("auto")
        ; pos != std::string::npos
        ; pos = type_name.find("auto", pos+1)) {

        if (pos != 0) {
            if (std::isalnum(type_name[pos-1]) || type_name[pos-1] == '_') {
                continue;
            }
        }

        if (pos + 3/*pos of 'o'*/ < type_name.size()-1) {
            if (std::isalnum(type_name[pos+3+1]) || type_name[pos+3+1] == '_') {
                continue;
            }
        }

        return true;
    }

    return false;
}

CXChildVisitResult unexposed_type_deducer(CXCursor cursor, CXCursor, CXClientData data)
{
    auto const type = clang_getCursorType(cursor);
    auto const type_name = owned(clang_getTypeSpelling(type));
    if (type.kind == CXType_Invalid || is_auto_type(to_c_str(type_name))) {
        clang_visitChildren(cursor, unexposed_type_deducer, data);
        return CXChildVisit_Continue;
    } else {
        *(reinterpret_cast<CXType *>(data)) = type;
        return CXChildVisit_Break;
    }
}

CXType deduce_type_at_cursor(CXCursor const& cursor)
{
    auto const type = clang_getCursorType(cursor);
    auto const type_name = owned(clang_getTypeSpelling(type));
    if (type.kind == CXType_Invalid || is_auto_type(to_c_str(type_name))) {
        CXType deduced_type;
        deduced_type.kind = CXType_Invalid;
        clang_visitChildren(cursor, unexposed_type_deducer, &deduced_type);
        return deduced_type;
    } else {
        return type;
    }
}

} // namespace detail

template<class LocationTuple>
inline char const* deduce_var_decl_type(LocationTuple const& location_tuple)
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

                    CXType const var_type = detail::deduce_type_at_cursor(var_decl_cursor);
                    if (var_type.kind == CXType_Invalid) {
                        return "{}";
                    }

                    std::string result;
                    result += stringize_type(var_type);
                    result += "'canonical':{" + stringize_type(clang_getCanonicalType(var_type)) + "},";
                    return "{" + result + "}";
                }
            );
}

namespace detail {

CXType deduce_func_decl_type_at_cursor(CXCursor const& cursor)
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

            CXType deduced_type;
            deduced_type.kind = CXType_Invalid;
            clang_visitChildren(return_stmt_cursor, unexposed_type_deducer, &deduced_type);
            return deduced_type;
        }
        default: return result_type;
    }
}

} // namespace detail

template<class LocationTuple>
inline char const* deduce_func_return_type(LocationTuple const& location_tuple)
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

                    CXType const func_type = detail::deduce_func_decl_type_at_cursor(func_decl_cursor);
                    if (func_type.kind == CXType_Invalid) {
                        return "{}";
                    }

                    std::string result;
                    result += stringize_type(func_type);
                    result += "'canonical':{" + stringize_type(clang_getCanonicalType(func_type)) + "},";
                    return "{" + result + "}";
                }
            );
}

template<class LocationTuple>
inline char const* deduce_func_or_var_decl(LocationTuple const& location_tuple)
{
    return at_specific_location(
                location_tuple,
                [](CXCursor const& cursor)
                    -> std::string
                {
                    CXCursor const func_or_var_decl = search_kind(
                            cursor,
                            [](CXCursorKind const& kind){
                                return kind == CXCursor_VarDecl || is_function_decl_kind(kind);
                            });
                    if (clang_Cursor_isNull(func_or_var_decl)) {
                        return "{}";
                    }

                    CXType const result_type =
                        clang_getCursorKind(cursor) == CXCursor_VarDecl ?
                            detail::deduce_type_at_cursor(func_or_var_decl) :
                            detail::deduce_func_decl_type_at_cursor(func_or_var_decl);
                    if (result_type.kind == CXType_Invalid) {
                        return "{}";
                    }

                    std::string result;
                    result += stringize_type(result_type);
                    result += "'canonical':{" + stringize_type(clang_getCanonicalType(result_type)) + "},";
                    return "{" + result + "}";
                }
            );
}

namespace detail {

CXChildVisitResult valid_type_cursor_getter(CXCursor cursor, CXCursor, CXClientData data)
{
    auto const type = clang_getCursorType(cursor);
    if (type.kind != CXType_Invalid) {
        *(reinterpret_cast<CXCursor *>(data)) = cursor;
        return CXChildVisit_Break;
    }
    return CXChildVisit_Recurse;
}

inline bool is_invalid_type_cursor(CXCursor const& cursor)
{
    return clang_getCursorType(cursor).kind == CXType_Invalid;
}

} // namespace detail

template<class LocationTuple>
inline char const* deduce_type_at(LocationTuple const& location_tuple)
{
    return at_specific_location(
                location_tuple,
                [](CXCursor const& cursor)
                    -> std::string
                {
                    CXCursor valid_cursor = cursor;
                    if (detail::is_invalid_type_cursor(valid_cursor)) {
                        clang_visitChildren(cursor, detail::valid_type_cursor_getter, &valid_cursor);
                    }
                    if (detail::is_invalid_type_cursor(valid_cursor)) {
                        return "{}";
                    }

                    CXCursorKind const kind = clang_getCursorKind(valid_cursor);
                    CXType const result_type =
                        kind == CXCursor_VarDecl ?
                            detail::deduce_type_at_cursor(valid_cursor) :
                            is_function_decl_kind(kind) ?
                                detail::deduce_func_decl_type_at_cursor(valid_cursor) :
                                clang_getCursorType(valid_cursor);
                    if (result_type.kind == CXType_Invalid) {
                        return "{}";
                    }

                    std::string result;
                    result += stringize_type(result_type);
                    result += "'canonical':{" + stringize_type(clang_getCanonicalType(result_type)) + "},";
                    return "{" + result + "}";
                }
            );
}

} // namespace libclang_vim

#endif    // LIBCLANG_VIM_DEDUCTION_HPP_INCLUDED

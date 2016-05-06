#if !defined LIBCLANG_VIM_DEDUCTION_HPP_INCLUDED
#define      LIBCLANG_VIM_DEDUCTION_HPP_INCLUDED

#include <cctype>
#include <string>
#include <stack>
#include <set>

#include "helpers.hpp"
#include "stringizers.hpp"

namespace libclang_vim {

namespace detail {

inline bool is_auto_type(std::string const& type_name)
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

inline CXChildVisitResult unexposed_type_deducer(CXCursor cursor, CXCursor, CXClientData data)
{
    auto const type = clang_getCursorType(cursor);
    cxstring_ptr type_name = clang_getTypeSpelling(type);
    if (type.kind == CXType_Invalid || is_auto_type(to_c_str(type_name))) {
        clang_visitChildren(cursor, unexposed_type_deducer, data);
        return CXChildVisit_Continue;
    } else {
        *(reinterpret_cast<CXType *>(data)) = type;
        return CXChildVisit_Break;
    }
}

inline CXType deduce_type_at_cursor(CXCursor const& cursor)
{
    auto const type = clang_getCursorType(cursor);
    cxstring_ptr type_name = clang_getTypeSpelling(type);
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

inline char const* deduce_var_decl_type(const location_tuple& location_info)
{
    return at_specific_location(
                location_info,
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

inline CXType deduce_func_decl_type_at_cursor(CXCursor const& cursor)
{
    auto const func_type = clang_getCursorType(cursor);
    auto const result_type = clang_getResultType(func_type);

    switch (result_type.kind) {
        case CXType_Unexposed: {
            cxstring_ptr type_name = clang_getTypeSpelling(result_type);
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

inline char const* deduce_func_return_type(const location_tuple& location_info)
{
    return at_specific_location(
                location_info,
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

inline char const* deduce_func_or_var_decl(const location_tuple& location_info)
{
    return at_specific_location(
                location_info,
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

inline CXChildVisitResult valid_type_cursor_getter(CXCursor cursor, CXCursor, CXClientData data)
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

inline char const* deduce_type_at(const location_tuple& location_info)
{
    return at_specific_location(
                location_info,
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

/// Wrapper around clang_getCursorSpelling().
const char* get_current_function_at(const location_tuple& location_info);

/// Wrapper around clang_Cursor_getBriefCommentText().
const char* get_comment_at(const location_tuple& location_info);

/// Get location of declaration referenced by location_info.
const char* get_deduced_declaration_at(const location_tuple& location_info);

/// Wrapper around clang_getIncludedFile().
const char* get_include_at(const location_tuple& location_info);

/// Wrapper around clang_codeCompleteAt().
const char* get_completion_at(const location_tuple& location_info);

/// Wrapper around clang_CompilationDatabase_getCompileCommands().
const char* get_compile_commands(const std::string& file);

/// Wrapper around clang_getDiagnostic().
const char* get_diagnostics(const std::pair<std::string, args_type>& file_and_args);

} // namespace libclang_vim

#endif    // LIBCLANG_VIM_DEDUCTION_HPP_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

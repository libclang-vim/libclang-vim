#if !defined LIBCLANG_VIM_DEDUCTION_HPP_INCLUDED
#define LIBCLANG_VIM_DEDUCTION_HPP_INCLUDED

#include <cctype>
#include <string>
#include <stack>
#include <set>

#include "stringizers.hpp"

namespace libclang_vim {

const char* deduce_var_decl_type(const location_tuple& location_info);

const char* deduce_func_return_type(const location_tuple& location_info);

const char* deduce_func_or_var_decl(const location_tuple& location_info);

/// Get type at specific location with auto-deduction described above.
const char* deduce_type_at(const location_tuple& location_info);

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
const char* get_diagnostics(const location_tuple& location_info);

} // namespace libclang_vim

#endif // LIBCLANG_VIM_DEDUCTION_HPP_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

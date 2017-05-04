#if !defined LIBCLANG_VIM_AST_EXTRACTER_HPP_INCLUDED
#define LIBCLANG_VIM_AST_EXTRACTER_HPP_INCLUDED

#include <string>
#include <tuple>

#include <clang-c/Index.h>

#include "helpers.hpp"
#include "stringizers.hpp"

namespace libclang_vim {

enum struct extraction_policy {
    all = 0,
    non_system_headers,
    current_file,
};

const char*
extract_AST_nodes(char const* arguments, extraction_policy policy,
                  const std::function<bool(const CXCursor&)>& predicate);

} // namespace libclang_vim

#endif // LIBCLANG_VIM_AST_EXTRACTER_HPP_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

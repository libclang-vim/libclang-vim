#if !defined LIBCLANG_VIM_STRINGIZERS_HPP_INCLUDED
#define LIBCLANG_VIM_STRINGIZERS_HPP_INCLUDED

#include <string>

#include <clang-c/Index.h>

#include "helpers.hpp"

namespace libclang_vim {

std::string stringize_spell(CXCursor const& cursor);

std::string stringize_extra_type_info(CXType const& type);

std::string stringize_type(CXType const& type);

std::string stringize_linkage_kind(CXLinkageKind const& linkage);

std::string stringize_linkage(CXCursor const& cursor);

std::string stringize_parent(CXCursor const& cursor, CXCursor const& parent);

std::string stringize_location(CXSourceLocation const& location);

std::string stringize_cursor_location(CXCursor const& cursor);

std::string stringize_cursor_kind_type(CXCursorKind const& kind);

std::string stringize_cursor_extra_info(CXCursor const& cursor);

std::string stringize_cursor_kind(CXCursor const& cursor);

std::string stringize_included_file(CXCursor const& cursor);

std::string stringize_cursor(CXCursor const& cursor, CXCursor const& parent);

std::string stringize_range(CXSourceRange const& range);

std::string stringize_extent(CXCursor const& cursor);

} // namespace libclang_vim

#endif // LIBCLANG_VIM_STRINGIZERS_HPP_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

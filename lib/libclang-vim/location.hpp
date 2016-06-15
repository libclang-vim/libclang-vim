#if !defined LIBCLANG_VIM_LOCATION_HPP_INCLUDED
#define LIBCLANG_VIM_LOCATION_HPP_INCLUDED

#include <tuple>
#include <string>
#include <cstdio>

#include <clang-c/Index.h>

#include "helpers.hpp"
#include "stringizers.hpp"

namespace libclang_vim {

const char* get_extent(const location_tuple& location_info,
                       const std::function<unsigned(CXCursor)>& predicate);

const char*
get_related_node_of(const location_tuple& location_info,
                    const std::function<CXCursor(CXCursor)>& predicate);

const char* get_type_related_to(const location_tuple& location_info,
                                const std::function<CXType(CXType)>& predicate);

const char* get_all_extents(const location_tuple& location_info);

} // namespace libclang_vim

#endif // LIBCLANG_VIM_LOCATION_HPP_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

#if !defined LIBCLANG_VIM_AST_EXTRACTER_HPP_INCLUDED
#define LIBCLANG_VIM_AST_EXTRACTER_HPP_INCLUDED

#include <tuple>
#include <string>

#include <clang-c/Index.h>

#include "helpers.hpp"
#include "stringizers.hpp"

namespace libclang_vim {

enum struct extraction_policy {
    all = 0,
    non_system_headers,
    current_file,
};

namespace detail {

enum { result = 0, visit_policy, predicate };

typedef std::tuple<std::string&, extraction_policy const,
                   const std::function<bool(const CXCursor&)>&>
    callback_data_type;

CXChildVisitResult AST_extracter(CXCursor cursor, CXCursor parent,
                                 CXClientData data) {
    auto& callback_data = *reinterpret_cast<callback_data_type*>(data);
    auto& vimson = std::get<result>(callback_data);
    auto& policy = std::get<visit_policy>(callback_data);

    if (policy == extraction_policy::current_file) {
        auto const location = clang_getCursorLocation(cursor);
        if (!clang_Location_isFromMainFile(location)) {
            return CXChildVisit_Continue;
        }
    }

    if (policy == extraction_policy::non_system_headers) {
        auto const location = clang_getCursorLocation(cursor);
        if (clang_Location_isInSystemHeader(location)) {
            return CXChildVisit_Continue;
        }
    }

    bool const is_target_node = std::get<predicate>(callback_data)(cursor);
    if (is_target_node) {
        vimson += "{" + stringize_cursor(cursor, parent) + "'children':[";
    }

    // visit children recursively
    clang_visitChildren(cursor, AST_extracter, data);

    if (is_target_node) {
        vimson += "]},";
    }

    return CXChildVisit_Continue;
}

} // namespace detail

const char*
extract_AST_nodes(char const* arguments, extraction_policy const policy,
                  const std::function<bool(const CXCursor&)>& predicate) {
    static std::string vimson;
    vimson = "";

    auto const parsed = parse_default_args(arguments);
    auto const args_ptrs = get_args_ptrs(parsed.args);

    detail::callback_data_type callback_data{vimson, policy, predicate};

    cxindex_ptr index =
        clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/ 0);
    cxtranslation_unit_ptr translation_unit(clang_parseTranslationUnit(
        index, parsed.file.c_str(), args_ptrs.data(), args_ptrs.size(), nullptr,
        0, CXTranslationUnit_Incomplete));
    if (!translation_unit)
        return "{}";

    CXCursor cursor = clang_getTranslationUnitCursor(translation_unit);
    clang_visitChildren(cursor, detail::AST_extracter, &callback_data);

    vimson = "{'root':[" + vimson + "]}";

    return vimson.c_str();
}

} // namespace libclang_vim

#endif // LIBCLANG_VIM_AST_EXTRACTER_HPP_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

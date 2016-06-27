#include "AST_extracter.hpp"

namespace {

enum { result = 0, visit_policy, predicate };

using callback_data_type =
    std::tuple<std::string&, libclang_vim::extraction_policy const,
               const std::function<bool(const CXCursor&)>&>;

CXChildVisitResult AST_extracter(CXCursor cursor, CXCursor parent,
                                 CXClientData data) {
    auto& callback_data = *reinterpret_cast<callback_data_type*>(data);
    auto& vimson = std::get<result>(callback_data);
    auto& policy = std::get<visit_policy>(callback_data);

    if (policy == libclang_vim::extraction_policy::current_file) {
        auto const location = clang_getCursorLocation(cursor);
        if (!clang_Location_isFromMainFile(location)) {
            return CXChildVisit_Continue;
        }
    }

    if (policy == libclang_vim::extraction_policy::non_system_headers) {
        auto const location = clang_getCursorLocation(cursor);
        if (clang_Location_isInSystemHeader(location)) {
            return CXChildVisit_Continue;
        }
    }

    bool const is_target_node = std::get<predicate>(callback_data)(cursor);
    if (is_target_node) {
        vimson += "{" + libclang_vim::stringize_cursor(cursor, parent) +
                  "'children':[";
    }

    // visit children recursively
    clang_visitChildren(cursor, AST_extracter, data);

    if (is_target_node) {
        vimson += "]},";
    }

    return CXChildVisit_Continue;
}
}

const char* libclang_vim::extract_AST_nodes(
    char const* arguments, extraction_policy const policy,
    const std::function<bool(const CXCursor&)>& predicate) {
    static std::string vimson;
    vimson = "";

    auto const parsed = parse_default_args(arguments);
    auto const args_ptrs = get_args_ptrs(parsed.args);

    callback_data_type callback_data{vimson, policy, predicate};

    cxindex_ptr index =
        clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/ 0);
    cxtranslation_unit_ptr translation_unit(clang_parseTranslationUnit(
        index, parsed.file.c_str(), args_ptrs.data(), args_ptrs.size(), nullptr,
        0, CXTranslationUnit_Incomplete));
    if (!translation_unit)
        return "{}";

    CXCursor cursor = clang_getTranslationUnitCursor(translation_unit);
    clang_visitChildren(cursor, AST_extracter, &callback_data);

    vimson = "{'root':[" + vimson + "]}";

    return vimson.c_str();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

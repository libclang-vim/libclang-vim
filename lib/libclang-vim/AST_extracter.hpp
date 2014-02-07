#if !defined LIBCLANG_VIM_AST_EXTRACTER_HPP_INCLUDED
#define      LIBCLANG_VIM_AST_EXTRACTER_HPP_INCLUDED

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

    enum {
        result = 0, visit_policy, predicate
    };

    template<class CallbackData>
    CXChildVisitResult AST_extracter(CXCursor cursor, CXCursor parent, CXClientData data)
    {
        auto &callback_data = *reinterpret_cast<CallbackData *>(data);
        auto &vimson = std::get<result>(callback_data);
        auto &policy = std::get<visit_policy>(callback_data);

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
        clang_visitChildren(cursor, AST_extracter<CallbackData>, data);

        if (is_target_node) {
            vimson += "]},";
        }

        return CXChildVisit_Continue;
    }

} // namespace detail

template<class Predicate>
auto extract_AST_nodes(
        char const* arguments,
        extraction_policy const policy,
        Predicate const& predicate
    ) -> char const*
{
    static std::string vimson;
    vimson = "";

    auto const parsed = parse_default_args(arguments);
    auto const args_ptrs = get_args_ptrs(parsed.second);

    typedef std::tuple<std::string&, extraction_policy const, Predicate const&> callback_data_type;
    callback_data_type callback_data{vimson, policy, predicate};

    CXIndex index = clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/0);
    CXTranslationUnit translation_unit = clang_parseTranslationUnit(index, parsed.first.c_str(), args_ptrs.data(), args_ptrs.size(), NULL, 0, CXTranslationUnit_Incomplete);
    if (translation_unit == NULL) {
        clang_disposeIndex(index);
        return "{}";
    }

    CXCursor cursor = clang_getTranslationUnitCursor(translation_unit);
    clang_visitChildren(cursor, detail::AST_extracter<callback_data_type>, &callback_data);

    clang_disposeTranslationUnit(translation_unit);
    clang_disposeIndex(index);

    vimson = "{'root':[" + vimson + "]}";

    return vimson.c_str();
}

} // namespace libclang_vim

#endif    // LIBCLANG_VIM_AST_EXTRACTER_HPP_INCLUDED

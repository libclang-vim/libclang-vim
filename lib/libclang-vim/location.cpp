#include "location.hpp"

namespace {

CXCursor search_AST_upward(CXCursor cursor,
                           const std::function<unsigned(CXCursor)>& predicate) {
    while (!clang_isInvalid(clang_getCursorKind(cursor))) {
        if (predicate(cursor)) {
            return cursor;
        }
        cursor = clang_getCursorSemanticParent(cursor);
    }
    return clang_getNullCursor();
}
}

const char*
libclang_vim::get_extent(const libclang_vim::location_tuple& location_info,
                         const std::function<unsigned(CXCursor)>& predicate) {
    return at_specific_location(
        location_info, [&predicate](const CXCursor& c) -> std::string {
            const CXCursor rc = search_AST_upward(c, predicate);
            if (clang_Cursor_isNull(rc)) {
                return "{}";
            }
            return "{" + stringize_extent(rc) + "}";
        });
};

const char* libclang_vim::get_related_node_of(
    const libclang_vim::location_tuple& location_info,
    const std::function<CXCursor(CXCursor)>& predicate) {
    return at_specific_location(
        location_info, [&predicate](CXCursor const& c) -> std::string {
            CXCursor const rc = predicate(c);
            if (clang_isInvalid(clang_getCursorKind(rc))) {
                return "{}";
            }
            return "{" +
                   stringize_cursor(rc, clang_getCursorSemanticParent(rc)) +
                   "}";
        });
}

const char* libclang_vim::get_type_related_to(
    const libclang_vim::location_tuple& location_info,
    const std::function<CXType(CXType)>& predicate) {
    return at_specific_location(
        location_info, [&predicate](CXCursor const& c) -> std::string {
            CXType const type = predicate(clang_getCursorType(c));
            if (type.kind == CXType_Invalid) {
                return "{}";
            }
            return "{" + stringize_type(type) + "}";
        });
}

const char* libclang_vim::get_all_extents(
    const libclang_vim::location_tuple& location_info) {
    static std::string vimson;
    vimson = "";
    char const* file_name = location_info.file.c_str();

    cxindex_ptr index =
        clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/ 0);
    auto const args_ptrs = get_args_ptrs(location_info.args);
    std::vector<CXUnsavedFile> unsaved_files =
        create_unsaved_files(location_info);
    unsigned options = CXTranslationUnit_Incomplete;
    cxtranslation_unit_ptr translation_unit(clang_parseTranslationUnit(
        index, file_name, args_ptrs.data(), args_ptrs.size(),
        unsaved_files.data(), unsaved_files.size(), options));
    if (!translation_unit)
        return "[]";

    CXFile file = clang_getFile(translation_unit, file_name);
    auto const location = clang_getLocation(
        translation_unit, file, location_info.line, location_info.col);
    CXCursor cursor = clang_getCursor(translation_unit, location);
    vimson += "{" + stringize_extent(cursor) + "},";

    bool already_pass_expression = false, already_pass_statement = false;
    while (!clang_isInvalid(clang_getCursorKind(cursor))) {
        if (is_class_decl(cursor) || is_function_decl(cursor) ||
            clang_getCursorKind(cursor) == CXCursor_Namespace ||
            (!already_pass_expression &&
             clang_isExpression(clang_getCursorKind(cursor))) ||
            (!already_pass_statement &&
             clang_isStatement(clang_getCursorKind(cursor)))) {
            vimson += "{" + stringize_extent(cursor) + "},";
        }
        cursor = clang_getCursorSemanticParent(cursor);
    }

    vimson = "[" + vimson + "]";

    return vimson.c_str();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

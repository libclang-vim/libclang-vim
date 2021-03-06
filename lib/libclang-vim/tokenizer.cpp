#include "tokenizer.hpp"
#include <numeric>

CXSourceRange libclang_vim::tokenizer::get_range_whole_file(
    const location_tuple& tuple,
    const libclang_vim::cxtranslation_unit_ptr& translation_unit) const {
    size_t const file_size = tuple.unsaved_file.empty()
                                 ? get_file_size(tuple.file.c_str())
                                 : tuple.unsaved_file.size();
    CXFile file = clang_getFile(translation_unit, tuple.file.c_str());

    auto const file_begin =
        clang_getLocationForOffset(translation_unit, file, 0);
    auto const file_end =
        clang_getLocationForOffset(translation_unit, file, file_size);

    if (is_null_location(file_begin) || is_null_location(file_end)) {
        return clang_getNullRange();
    }

    auto const file_range = clang_getRange(file_begin, file_end);
    if (clang_Range_isNull(file_range)) {
        return clang_getNullRange();
    }

    return file_range;
}

const char*
libclang_vim::tokenizer::get_kind_spelling(const CXTokenKind kind) const {
    switch (kind) {
    case CXToken_Punctuation:
        return "punctuation";
    case CXToken_Keyword:
        return "keyword";
    case CXToken_Identifier:
        return "identifier";
    case CXToken_Literal:
        return "literal";
    case CXToken_Comment:
        return "comment";
    }
}

std::string libclang_vim::tokenizer::make_vimson_from_tokens(
    const libclang_vim::cxtranslation_unit_ptr& translation_unit,
    std::vector<CXToken> tokens) const {
    return "[" + std::accumulate(
                     std::begin(tokens), std::end(tokens), std::string{},
                     [&](std::string const& acc, CXToken const& token) {
                         auto const kind = clang_getTokenKind(token);
                         cxstring_ptr spell =
                             clang_getTokenSpelling(translation_unit, token);
                         auto const location =
                             clang_getTokenLocation(translation_unit, token);

                         CXFile file;
                         unsigned int line, column, offset;
                         clang_getFileLocation(location, &file, &line, &column,
                                               &offset);
                         cxstring_ptr source_name = clang_getFileName(file);

                         return acc + "{'spell':'" + to_c_str(spell) +
                                "','kind':'" + get_kind_spelling(kind) +
                                "','file':'" + clang_getCString(source_name) +
                                "','line':" + std::to_string(line) +
                                ",'column':" + std::to_string(column) +
                                ",'offset':" + std::to_string(offset) + "},";
                     }) +
           "]";
}

std::string
libclang_vim::tokenizer::tokenize_as_vimson(const location_tuple& tuple) {
    cxindex_ptr index =
        clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/ 0);

    std::vector<const char*> args_ptrs = get_args_ptrs(tuple.args);
    std::vector<CXUnsavedFile> unsaved_files = create_unsaved_files(tuple);
    unsigned options = CXTranslationUnit_Incomplete;
    cxtranslation_unit_ptr translation_unit = clang_parseTranslationUnit(
        index, tuple.file.c_str(), args_ptrs.data(), args_ptrs.size(),
        unsaved_files.data(), unsaved_files.size(), options);
    if (!translation_unit)
        return "{}";

    auto file_range = get_range_whole_file(tuple, translation_unit);
    if (clang_Range_isNull(file_range))
        return "{}";

    CXToken* tokens_;
    unsigned int num_tokens;
    clang_tokenize(translation_unit, file_range, &tokens_, &num_tokens);
    std::vector<CXToken> tokens(tokens_, tokens_ + num_tokens);

    auto result = make_vimson_from_tokens(translation_unit, tokens);

    clang_disposeTokens(translation_unit, tokens_, num_tokens);

    return result;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

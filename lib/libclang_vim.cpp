#include <cstddef>
#include <cstdlib>
#include <string>
#include <fstream>
#include <vector>
#include <numeric>
#include <clang-c/Index.h>

using std::size_t;

#include <iostream>

// Note: boost::filesystem
inline size_t get_file_size(char const* filename)
{
    std::ifstream input(filename);
    return input.seekg(0, std::ios::end).tellg();
}

template<class Location>
inline bool is_null_location(Location const& location)
{
    return clang_equalLocations(location, clang_getNullLocation());
}

CXSourceRange get_range_whole_file(char const* file_name, CXTranslationUnit const& translation_unit)
{
    size_t const file_size = get_file_size(file_name);
    CXFile const file = clang_getFile(translation_unit, file_name);

    auto const file_begin = clang_getLocationForOffset(translation_unit, file, 0);
    auto const file_end = clang_getLocationForOffset(translation_unit, file, file_size);

    if(is_null_location(file_begin) || is_null_location(file_end)) {
        std::cerr << "location cannot be obtained\n";
        return clang_getNullRange();
    }

    auto const file_range = clang_getRange(file_begin, file_end);
    if(clang_Range_isNull(file_range)) {
        std::cerr << "range cannot be obtained\n";
        return clang_getNullRange();
    }

    return file_range;
}

template<class Kind>
inline char const* get_kind_spelling(Kind const kind)
{
    switch (kind) {
        case CXToken_Punctuation: return "punctuation";
        case CXToken_Keyword:     return "keyword";
        case CXToken_Identifier:  return "identifier";
        case CXToken_Literal:     return "literal";
        case CXToken_Comment:     return "comment";
        default:             return "unknown";
    }
}

template<class Token>
std::string make_vimson_from_tokens(CXTranslationUnit const& translation_unit, std::vector<Token> tokens)
{
    return "[" + std::accumulate(std::begin(tokens), std::end(tokens), std::string{}, [&](std::string const& acc, Token const& token){
        auto const kind = clang_getTokenKind(token);
        auto const spell = clang_getTokenSpelling(translation_unit, token);
        auto const location = clang_getTokenLocation(translation_unit, token);

        CXFile file;
        unsigned int line, column, offset;
        clang_getFileLocation(location, &file, &line, &column, &offset);
        auto const file_name = clang_getFileName(file);

        auto const result = acc
            + "{'spell':'" + clang_getCString(spell)
            + "','kind':'" + get_kind_spelling(kind)
            + "','file':'" + clang_getCString(file_name)
            + "','line':'" + std::to_string(line)
            + "','column':'" + std::to_string(column)
            + "','offset':'" + std::to_string(offset)
            + "'},";

        clang_disposeString(file_name);
        clang_disposeString(spell);

        return result;
    }) + "]";
}

std::string make_vimson_tokens_from_file(char const* file_name, size_t const argc, char const* args[])
{
    CXIndex index = clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/0);

    CXTranslationUnit translation_unit = clang_parseTranslationUnit(index, file_name, args, argc, NULL, 0, CXTranslationUnit_Incomplete);
    auto file_range = get_range_whole_file(file_name, translation_unit);
    if (clang_Range_isNull(file_range)) {
        std::cerr << "cannot get range to parse\n";
        return "";
    }

    if (!translation_unit) {
        std::cerr << "cannot parse translation unit\n";
        return "";
    }

    CXToken *tokens_;
    unsigned int num_tokens;
    clang_tokenize(translation_unit, file_range, &tokens_, &num_tokens);
    std::vector<CXToken> tokens(tokens_, tokens_ + num_tokens);

    auto result = make_vimson_from_tokens(translation_unit, tokens);

    clang_disposeTokens(translation_unit, tokens_, num_tokens);
    clang_disposeTranslationUnit(translation_unit);
    clang_disposeIndex(index);

    return result;
}

#ifdef __cplusplus
extern "C" {
#endif

char const* vim_clang_version()
{
    CXString version = clang_getClangVersion();
    return clang_getCString(version);
}

char const* vim_clang_tokens(char const* file_name)
{
    static auto const vimson = make_vimson_tokens_from_file(file_name, 0, {});
    return vimson == "" ? NULL : vimson.c_str();
}

#ifdef __cplusplus
} // extern "C"
#endif

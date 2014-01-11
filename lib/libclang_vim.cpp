#include <cstddef>
#include <cstdlib>
#include <string>
#include <fstream>
#include <vector>
#include <numeric>
#include <clang-c/Index.h>

using std::size_t;

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

// Tokenizer {{{
class clang_vimson_tokenizer {
private:
    CXTranslationUnit translation_unit;
    char const* file_name;

public:
    clang_vimson_tokenizer(char const* file_name)
        : translation_unit(), file_name(file_name)
    {}

private:
    CXSourceRange get_range_whole_file() const
    {
        size_t const file_size = get_file_size(file_name);
        CXFile const file = clang_getFile(translation_unit, file_name);

        auto const file_begin = clang_getLocationForOffset(translation_unit, file, 0);
        auto const file_end = clang_getLocationForOffset(translation_unit, file, file_size);

        if(is_null_location(file_begin) || is_null_location(file_end)) {
            return clang_getNullRange();
        }

        auto const file_range = clang_getRange(file_begin, file_end);
        if(clang_Range_isNull(file_range)) {
            return clang_getNullRange();
        }

        return file_range;
    }

    template<class Kind>
    inline char const* get_kind_spelling(Kind const kind) const
    {
        switch (kind) {
            case CXToken_Punctuation: return "punctuation";
            case CXToken_Keyword:     return "keyword";
            case CXToken_Identifier:  return "identifier";
            case CXToken_Literal:     return "literal";
            case CXToken_Comment:     return "comment";
            default:                  return "unknown";
        }
    }

    template<class Token>
    std::string make_vimson_from_tokens(std::vector<Token> tokens) const
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

public:
    std::string tokenize_as_vimson(size_t const argc, char const* args[])
    {
        CXIndex index = clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/0);

        translation_unit = clang_parseTranslationUnit(index, file_name, args, argc, NULL, 0, CXTranslationUnit_Incomplete);
        if (translation_unit == NULL) {
            return "";
        }
        auto file_range = get_range_whole_file();
        if (clang_Range_isNull(file_range)) {
            return "";
        }

        CXToken *tokens_;
        unsigned int num_tokens;
        clang_tokenize(translation_unit, file_range, &tokens_, &num_tokens);
        std::vector<CXToken> tokens(tokens_, tokens_ + num_tokens);

        auto result = make_vimson_from_tokens(tokens);

        clang_disposeTokens(translation_unit, tokens_, num_tokens);
        clang_disposeTranslationUnit(translation_unit);
        clang_disposeIndex(index);

        return result;
    }

};
// }}}

#include <iostream>

class clang_vimson_AST_builder;

namespace detail {

CXChildVisitResult visit_AST(CXCursor cursor, CXCursor parent, CXClientData data)
{
    auto *this_ = reinterpret_cast<clang_vimson_AST_builder *>(data);
    clang_visitChildren(cursor, visit_AST, this_);
    return CXChildVisit_Continue;
}

} // namespace detail

class clang_vimson_AST_builder {
    CXTranslationUnit translation_unit;
    char const* file_name;
    std::string vimson_result;
public:
    clang_vimson_AST_builder(char const* file_name)
        : translation_unit(), file_name(file_name), vimson_result()
    {}

public:
    std::string build_as_vimson(int const argc, char const* argv[])
    {
        vimson_result = "";
        CXIndex index = clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/0);

        translation_unit = clang_parseTranslationUnit(index, file_name, argv, argc, NULL, 0, CXTranslationUnit_Incomplete);
        if (translation_unit == NULL) {
            return "";
        }

        auto cursor = clang_getTranslationUnitCursor(translation_unit);
        clang_visitChildren(cursor, detail::visit_AST, this);

        clang_disposeTranslationUnit(translation_unit);
        clang_disposeIndex(index);

        return vimson_result;
    }
};

int main()
{
    clang_vimson_AST_builder builder("tmp.cpp");
    std::cout << builder.build_as_vimson(0, {});

    return 0;
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
    clang_vimson_tokenizer tokenizer(file_name);
    static auto const vimson = tokenizer.tokenize_as_vimson(0, {});
    return vimson == "" ? NULL : vimson.c_str();
}

#ifdef __cplusplus
} // extern "C"
#endif

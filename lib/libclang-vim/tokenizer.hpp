#if !defined LIBCLANG_VIM_TOKENIZER_HPP_INCLUDED
#define      LIBCLANG_VIM_TOKENIZER_HPP_INCLUDED

#include <string>
#include <vector>
#include <numeric>

#include <clang-c/Index.h>

#include "helpers.hpp"

namespace libclang_vim {

class tokenizer {
private:
    CXTranslationUnit translation_unit;
    std::string file_name;

public:
    tokenizer(std::string const& file_name)
        : translation_unit()
        , file_name(file_name)
    {}

private:
    CXSourceRange get_range_whole_file() const
    {
        size_t const file_size = get_file_size(file_name.c_str());
        CXFile const file = clang_getFile(translation_unit, file_name.c_str());

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
            auto const spell = owned(clang_getTokenSpelling(translation_unit, token));
            auto const location = clang_getTokenLocation(translation_unit, token);

            CXFile file;
            unsigned int line, column, offset;
            clang_getFileLocation(location, &file, &line, &column, &offset);
            auto const source_name = owned(clang_getFileName(file));

            return acc
                + "{'spell':'" + clang_getCString(*spell)
                + "','kind':'" + get_kind_spelling(kind)
                + "','file':'" + clang_getCString(*source_name)
                + "','line':" + std::to_string(line)
                + ",'column':" + std::to_string(column)
                + ",'offset':" + std::to_string(offset)
                + "},";
        }) + "]";
    }

public:
    std::string tokenize_as_vimson(char const* const* args, size_t const argc)
    {
        CXIndex index = clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/0);

        translation_unit = clang_parseTranslationUnit(index, file_name.c_str(), args, argc, NULL, 0, CXTranslationUnit_Incomplete);
        if (translation_unit == NULL) {
            clang_disposeIndex(index);
            return "{}";
        }
        auto file_range = get_range_whole_file();
        if (clang_Range_isNull(file_range)) {
            clang_disposeTranslationUnit(translation_unit);
            clang_disposeIndex(index);
            return "{}";
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

} // namespace libclang_vim

#endif    // LIBCLANG_VIM_TOKENIZER_HPP_INCLUDED

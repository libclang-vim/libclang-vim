#include <cstddef>
#include <cstdlib>
#include <string>
#include <fstream>
#include <vector>
#include <numeric>
#include <memory>
#include <iostream>
#include <functional>

#include <clang-c/Index.h>

namespace libclang_vim {

using std::size_t;

// Helpers {{{
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

struct cxstring_deleter {
    void operator()(CXString *str) const
    {
        clang_disposeString(*str);
        delete str;
    }
};

typedef std::shared_ptr<CXString> cxstring_ptr;

inline cxstring_ptr owned(CXString const& str)
{
    return {new CXString(str), cxstring_deleter{}};
}

inline char const* to_c_str(cxstring_ptr const& p)
{
    return clang_getCString(*p);
}

inline std::string operator""_str(char const* s, size_t const)
{
    return {s};
}
// }}}

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
            auto const spell = owned(clang_getTokenSpelling(translation_unit, token));
            auto const location = clang_getTokenLocation(translation_unit, token);

            CXFile file;
            unsigned int line, column, offset;
            clang_getFileLocation(location, &file, &line, &column, &offset);
            auto const file_name = owned(clang_getFileName(file));

            return acc
                + "{'spell':'" + clang_getCString(*spell)
                + "','kind':'" + get_kind_spelling(kind)
                + "','file':'" + clang_getCString(*file_name)
                + "','line':'" + std::to_string(line)
                + "','column':'" + std::to_string(column)
                + "','offset':'" + std::to_string(offset)
                + "'},";
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

// Stringizers {{{
inline std::string stringize_spell(CXCursor const& cursor)
{
    auto const spell = owned(clang_getCursorSpelling(cursor));
    return "'spell':'"_str + to_c_str(spell) + "',";
}

inline std::string stringize_type(CXCursor const& cursor)
{
    CXType const type = clang_getCursorType(cursor);
    CXTypeKind const type_kind = type.kind;
    auto const type_name = owned(clang_getTypeSpelling(type));
    auto const type_kind_name = owned(clang_getTypeKindSpelling(type_kind));
    return "'type':'"_str + to_c_str(type_name) + "','type_kind':'" + to_c_str(type_kind_name) + "',";
}

inline std::string stringize_linkage_kind(CXLinkageKind const& linkage)
{
    switch (linkage) {
    case CXLinkage_Invalid: return "Invalid";
    case CXLinkage_NoLinkage: return "Nolinkage";
    case CXLinkage_Internal: return "Internal";
    case CXLinkage_UniqueExternal: return "UniqueExternal";
    case CXLinkage_External: return "External";
    default:                 return "Unknown";
    }
}

inline std::string stringize_linkage(CXCursor const& cursor)
{
    return "'linkage':'" + stringize_linkage_kind(clang_getCursorLinkage(cursor)) + "',";
}

inline std::string stringize_parent(CXCursor const& cursor, CXCursor const& parent)
{
    auto semantic_parent = clang_getCursorSemanticParent(cursor);
    auto lexical_parent = clang_getCursorLexicalParent(cursor);
    auto parent_name = owned(clang_getCursorSpelling(parent));
    auto semantic_parent_name = owned(clang_getCursorSpelling(semantic_parent));
    auto lexical_parent_name = owned(clang_getCursorSpelling(lexical_parent));

    return "'parent':'"_str + to_c_str(parent_name)
        + "','semantic_parent':'" + to_c_str(semantic_parent_name)
        + "','lexical_parent':'" + to_c_str(lexical_parent_name)
        + "',";
}

inline std::string stringize_location(CXCursor const& cursor)
{
    CXSourceLocation const location = clang_getCursorLocation(cursor);
    CXFile file;
    unsigned int line, column, offset;
    clang_getSpellingLocation(location, &file, &line, &column, &offset);
    auto const file_name = owned(clang_getFileName(file));

    return "'line':'" + std::to_string(line)
         + "','column':'" + std::to_string(column)
         + "','offset':'" + std::to_string(offset)
         + "','file':'" + to_c_str(file_name)
         + "',";
}

inline std::string stringize_USR(CXCursor const& cursor)
{
    auto USR = owned(clang_getCursorUSR(cursor));
    return "'USR':'"_str + to_c_str(USR) + "',";
}

inline char const* stringize_cursor_kind_type(CXCursorKind const& kind)
{
    if (clang_isAttribute(kind)) {
        return "Attribute";
    } else if (clang_isDeclaration(kind)) {
        return "Declaration";
    } else if (clang_isExpression(kind)) {
        return "Expression";
    } else if (clang_isInvalid(kind)) {
        return "Invalid";
    } else if (clang_isPreprocessing(kind)) {
        return "Preprocessing";
    } else if (clang_isReference(kind)) {
        return "Reference";
    } else if (clang_isStatement(kind)) {
        return "Statement";
    } else if (clang_isTranslationUnit(kind)) {
        return "TranslationUnit";
    } else if (clang_isUnexposed(kind)) {
        return "Unexposed";
    } else {
        return "Unknown";
    }
}

inline std::string stringize_cursor_kind(CXCursor const& cursor)
{
    CXCursorKind const kind = clang_getCursorKind(cursor);
    auto kind_name = owned(clang_getCursorKindSpelling(kind));

    return "'kind':'"_str + to_c_str(kind_name)
         + "','kind_type':'" + stringize_cursor_kind_type(kind)
         + "',";
}

inline std::string stringize_included_file(CXCursor const& cursor)
{
    CXFile const included_file = clang_getIncludedFile(cursor);
    if (included_file == NULL) {
        return "'included_file':'',";
    }

    auto included_file_name = owned(clang_getFileName(included_file));
    return "'included_file':'"_str + to_c_str(included_file_name) + "',";
}

inline std::string stringize_cursor(CXCursor const& cursor, CXCursor const& parent)
{
    return stringize_spell(cursor)
        + stringize_type(cursor)
        + stringize_linkage(cursor)
        + stringize_parent(cursor, parent)
        + stringize_location(cursor)
        + stringize_USR(cursor)
        + stringize_cursor_kind(cursor)
        + stringize_included_file(cursor);
}
// }}}

// AST builder {{{
class clang_vimson_AST_builder {
    char const* file_name;
    std::string vimson_result;

public:
    clang_vimson_AST_builder(char const* file_name)
        : file_name(file_name), vimson_result()
    {}

private:

    static CXChildVisitResult visit_AST(CXCursor cursor, CXCursor parent, CXClientData data)
    {
        auto *this_ = reinterpret_cast<clang_vimson_AST_builder *>(data);

        this_->vimson_result += "{" + stringize_cursor(cursor, parent) + "'children':[";

        clang_visitChildren(cursor, visit_AST, this_);

        this_->vimson_result += "]},";

        return CXChildVisit_Continue;
    }

public:
    std::string build_as_vimson(int const argc, char const* argv[])
    {
        vimson_result = "";
        CXIndex index = clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/0);

        CXTranslationUnit translation_unit = clang_parseTranslationUnit(index, file_name, argv, argc, NULL, 0, CXTranslationUnit_Incomplete);
        if (translation_unit == NULL) {
            return "";
        }

        auto cursor = clang_getTranslationUnitCursor(translation_unit);
        clang_visitChildren(cursor, visit_AST, this);

        clang_disposeTranslationUnit(translation_unit);
        clang_disposeIndex(index);

        vimson_result = "{'AST':[" + vimson_result + "]}";

        return vimson_result;
    }
};
// }}}

// TODO: :%s/clang_vimson//g
template<class Predicate>
class clang_vimson_kind_extracter {
    char const* file_name;
    Predicate predicate;
    std::string vimson_result;

public:
    clang_vimson_kind_extracter(char const* file_name, Predicate const& predicate)
        : file_name(file_name), predicate(predicate), vimson_result()
    {}

private:
    static CXChildVisitResult visit_AST(CXCursor cursor, CXCursor parent, CXClientData data)
    {
        auto *this_ = reinterpret_cast<clang_vimson_kind_extracter *>(data);

        if (!this_->predicate(clang_getCursorKind(cursor))) {
            return CXChildVisit_Continue;
        }

        this_->vimson_result += "{" + stringize_cursor(cursor, parent) + "},";

        clang_visitChildren(cursor, visit_AST, this_);

        return CXChildVisit_Continue;
    }

public:
    std::string extract_as_vimson(int const argc, char const* argv[])
    {
        vimson_result = "";
        CXIndex index = clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/0);

        CXTranslationUnit translation_unit = clang_parseTranslationUnit(index, file_name, argv, argc, NULL, 0, CXTranslationUnit_Incomplete);
        if (translation_unit == NULL) {
            return "";
        }

        auto cursor = clang_getTranslationUnitCursor(translation_unit);
        clang_visitChildren(cursor, visit_AST, this);

        clang_disposeTranslationUnit(translation_unit);
        clang_disposeIndex(index);

        vimson_result = "{'declarations':[" + vimson_result + "]}";
        return vimson_result;
    }
};

template<class Predicate>
inline auto make_clang_vimson_kind_extracter(char const* file_name, Predicate const& predicate)
    -> clang_vimson_kind_extracter<std::function<Predicate>>
{
    return {file_name, std::function<Predicate>{predicate}};
}

} // namespace libclang_vim

int main()
{
    auto extracter = libclang_vim::make_clang_vimson_kind_extracter("tmp.cpp", clang_isDeclaration);
    std::cout << extracter.extract_as_vimson(0, {});

    return 0;
}

extern "C" {

char const* vim_clang_version()
{
    auto version = libclang_vim::owned(clang_getClangVersion());
    return clang_getCString(*version);
}

char const* vim_clang_tokens(char const* file_name)
{
    libclang_vim::clang_vimson_tokenizer tokenizer(file_name);
    static auto const vimson = tokenizer.tokenize_as_vimson(0, {});
    return vimson == "" ? NULL : vimson.c_str();
}

char const* vim_clang_build_AST(char const* file_name)
{
    libclang_vim::clang_vimson_AST_builder builder(file_name);
    static auto const vimson = builder.build_as_vimson(0, {});
    return vimson == "" ? NULL : vimson.c_str();
}

char const* vim_clang_extract_declarations(char const* file_name)
{
    auto extracter = libclang_vim::make_clang_vimson_kind_extracter(file_name, clang_isDeclaration);
    static auto const vimson = extracter.extract_as_vimson(0, {});
    return vimson == "" ? NULL : vimson.c_str();
}

} // extern "C"

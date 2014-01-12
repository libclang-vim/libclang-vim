#include <cstddef>
#include <cstring>
#include <string>
#include <fstream>
#include <vector>
#include <numeric>
#include <memory>
#include <functional>
#include <utility>

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

inline std::string stringize_key_value(char const* key_name, cxstring_ptr const& p)
{
    auto const* cstring = clang_getCString(*p);
    if (!cstring || std::strcmp(cstring, "") == 0) {
        return "";
    } else {
        return "'" + std::string{key_name} + "':'" + cstring + "',";
    }
}

std::string stringize_key_value(char const* key_name, std::string const& s)
{
    if (s.empty()) {
        return "";
    } else {
        return "'" + (key_name + ("':'" + s + "',"));
    }
}

// }}}

// Tokenizer {{{
class tokenizer {
private:
    CXTranslationUnit translation_unit;
    char const* file_name;

public:
    tokenizer(char const* file_name)
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
    return stringize_key_value("spell", spell);
}

inline std::string stringize_extra_type_info(CXType const& type)
{
    std::string result = "";

    if (clang_isConstQualifiedType(type)) {
        result += "'is_const_qualified':1,";
    }
    if (clang_isVolatileQualifiedType(type)) {
        result += "'is_volatile_qualified':1,";
    }
    if (clang_isRestrictQualifiedType(type)) {
        result += "'is_restrict_qualified':1,";
    }
    if (clang_isPODType(type)) {
        result += "'is_POD_type':1,";
    }

    auto const ref_qualified = clang_Type_getCXXRefQualifier(type);
    switch (ref_qualified) {
    case CXRefQualifier_LValue: result += "'is_lvalue':1,"; break;
    case CXRefQualifier_RValue: result += "'is_rvalue':1,"; break;
    case CXRefQualifier_None: break;
    default: break;
    }

    // Note: The information about calling convention is really needed?
    //       If you want the information, please let me know that.
    //
    // auto const calling_convention = clang_getFunctionTypeCallingConv(type);
    // switch (calling_convention) {
    // ...

    return result;
}

inline std::string stringize_type(CXCursor const& cursor)
{
    CXType const type = clang_getCursorType(cursor);
    CXTypeKind const type_kind = type.kind;
    auto const type_name = owned(clang_getTypeSpelling(type));
    auto const type_kind_name = owned(clang_getTypeKindSpelling(type_kind));
    return stringize_key_value("type", type_name)
         + stringize_key_value("type_kind", type_kind_name)
         + stringize_extra_type_info(type);
}

inline std::string stringize_linkage_kind(CXLinkageKind const& linkage)
{
    switch (linkage) {
    case CXLinkage_Invalid: return "";
    case CXLinkage_NoLinkage: return "Nolinkage";
    case CXLinkage_Internal: return "Internal";
    case CXLinkage_UniqueExternal: return "UniqueExternal";
    case CXLinkage_External: return "External";
    default:                 return "Unknown";
    }
}

inline std::string stringize_linkage(CXCursor const& cursor)
{
    auto const linkage_kind_name = stringize_linkage_kind(clang_getCursorLinkage(cursor));
    return stringize_key_value("linkage", linkage_kind_name);
}

inline std::string stringize_parent(CXCursor const& cursor, CXCursor const& parent)
{
    auto const semantic_parent = clang_getCursorSemanticParent(cursor);
    auto const lexical_parent = clang_getCursorLexicalParent(cursor);
    auto const parent_name = owned(clang_getCursorSpelling(parent));
    auto const semantic_parent_name = owned(clang_getCursorSpelling(semantic_parent));
    auto const lexical_parent_name = owned(clang_getCursorSpelling(lexical_parent));

    return stringize_key_value("parent", parent_name)
         + stringize_key_value("semantic_parent", semantic_parent_name)
         + stringize_key_value("lexical_parent", lexical_parent_name);
}

inline std::string stringize_location(CXSourceLocation const& location)
{
    CXFile file;
    unsigned int line, column, offset;
    clang_getSpellingLocation(location, &file, &line, &column, &offset);
    auto const file_name = owned(clang_getFileName(file));

    return stringize_key_value("line", std::to_string(line))
         + stringize_key_value("column", std::to_string(column))
         + stringize_key_value("offset", std::to_string(offset))
         + stringize_key_value("file", file_name);
}

inline std::string stringize_cursor_location(CXCursor const& cursor)
{
    CXSourceLocation const location = clang_getCursorLocation(cursor);
    return stringize_location(location);
}

inline std::string stringize_USR(CXCursor const& cursor)
{
    auto USR = owned(clang_getCursorUSR(cursor));
    return stringize_key_value("USR", USR);
}

inline std::string stringize_cursor_kind_type(CXCursorKind const& kind)
{
    if (clang_isAttribute(kind)) {
        return "Attribute";
    } else if (clang_isDeclaration(kind)) {
        return "Declaration";
    } else if (clang_isExpression(kind)) {
        return "Expression";
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
    } else if (clang_isInvalid(kind)) {
        return "";
    } else {
        return "Unknown";
    }
}

inline std::string stringize_cursor_extra_info(CXCursor const& cursor)
{
    std::string result = "";

    if (clang_isCursorDefinition(cursor)) {
        result += "'is_definition':1,";
    }
    if (clang_Cursor_isDynamicCall(cursor)) {
        result += "'is_dynamic_call':1,";
    }
    if (clang_Cursor_isVariadic(cursor)) {
        result += "'is_variadic':1,";
    }
    if (clang_CXXMethod_isVirtual(cursor)) {
        result += "'is_virtual_member_function':1,";
    }
    if (clang_CXXMethod_isPureVirtual(cursor)) {
        result += "'is_pure_virtual_member_function':1,";
    }
    if (clang_CXXMethod_isStatic(cursor)) {
        result += "'is_static_member_function':1,";
    }

    auto const access_specifier = clang_getCXXAccessSpecifier(cursor);
    switch (access_specifier) {
    case CX_CXXPublic:    result += "'access_specifier':'public',"; break;
    case CX_CXXPrivate:   result += "'access_specifier':'private',"; break;
    case CX_CXXProtected: result += "'access_specifier':'protected',"; break;
    case CX_CXXInvalidAccessSpecifier: break;
    default: break;
    }

    return result;
}

inline std::string stringize_cursor_kind(CXCursor const& cursor)
{
    CXCursorKind const kind = clang_getCursorKind(cursor);
    auto const kind_name = owned(clang_getCursorKindSpelling(kind));
    auto const kind_type_name = stringize_cursor_kind_type(kind);

    return stringize_key_value("kind", kind_name)
         + (kind_type_name.empty() ? std::string{} : ("'kind_type':'" + kind_type_name + "',"))
         + stringize_cursor_extra_info(cursor);
}

inline std::string stringize_included_file(CXCursor const& cursor)
{
    CXFile const included_file = clang_getIncludedFile(cursor);
    if (included_file == NULL) {
        return "";
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
        + stringize_cursor_location(cursor)
        + stringize_USR(cursor)
        + stringize_cursor_kind(cursor)
        + stringize_included_file(cursor);
}
// }}}

// AST builder {{{
class AST_builder {
    std::string file_name;
    std::string vimson_result;

public:
    AST_builder(char const* file_name)
        : file_name(file_name), vimson_result()
    {}

private:
    static CXChildVisitResult visit_AST(CXCursor cursor, CXCursor parent, CXClientData data)
    {
        auto *this_ = reinterpret_cast<AST_builder *>(data);

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

        CXTranslationUnit translation_unit = clang_parseTranslationUnit(index, file_name.c_str(), argv, argc, NULL, 0, CXTranslationUnit_Incomplete);
        if (translation_unit == NULL) {
            return "";
        }

        CXCursor cursor = clang_getTranslationUnitCursor(translation_unit);
        if (clang_Cursor_isNull(cursor)) {
            return "";
        }

        clang_visitChildren(cursor, visit_AST, this);

        clang_disposeTranslationUnit(translation_unit);
        clang_disposeIndex(index);

        vimson_result = "{'AST':[" + vimson_result + "]}";

        return vimson_result;
    }
};
// }}}

// AST extracter {{{
template<class Predicate>
class AST_extracter {
    std::string file_name;
    Predicate predicate;
    std::string vimson_result;

public:
    AST_extracter(char const* file_name, Predicate const& predicate)
        : file_name(file_name), predicate(predicate), vimson_result()
    {}

private:
    static CXChildVisitResult visit_AST(CXCursor cursor, CXCursor parent, CXClientData data)
    {
        auto *this_ = reinterpret_cast<AST_extracter *>(data);

        if (!this_->predicate(cursor)) {
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

        CXTranslationUnit translation_unit = clang_parseTranslationUnit(index, file_name.c_str(), argv, argc, NULL, 0, CXTranslationUnit_Incomplete);
        if (translation_unit == NULL) {
            return "";
        }

        auto cursor = clang_getTranslationUnitCursor(translation_unit);
        clang_visitChildren(cursor, visit_AST, this);

        clang_disposeTranslationUnit(translation_unit);
        clang_disposeIndex(index);

        vimson_result = "[" + vimson_result + "]";
        return vimson_result;
    }
};

template<class Predicate>
inline auto make_AST_extracter(char const* file_name, Predicate const& predicate)
    -> AST_extracter<std::function<decltype(predicate(std::declval<CXCursor>()))(CXCursor const&)>>
{
    return {file_name, predicate};
}
// }}}

} // namespace libclang_vim

// C APIs {{{
extern "C" {

char const* vim_clang_version()
{
    auto version = libclang_vim::owned(clang_getClangVersion());
    return clang_getCString(*version);
}

char const* vim_clang_tokens(char const* file_name)
{
    libclang_vim::tokenizer tokenizer(file_name);
    static auto const vimson = tokenizer.tokenize_as_vimson(0, {});
    return vimson == "" ? NULL : vimson.c_str();
}

char const* vim_clang_build_AST(char const* file_name)
{
    libclang_vim::AST_builder builder(file_name);
    static auto const vimson = builder.build_as_vimson(0, {});
    return vimson == "" ? NULL : vimson.c_str();
}

char const* vim_clang_extract_declarations(char const* file_name)
{
    auto extracter = libclang_vim::make_AST_extracter(file_name, [](CXCursor const& c){ return clang_isDeclaration(clang_getCursorKind(c)); });
    static auto const vimson = extracter.extract_as_vimson(0, {});
    return vimson == "" ? NULL : vimson.c_str();
}

char const* vim_clang_extract_attributes(char const* file_name)
{
    auto extracter = libclang_vim::make_AST_extracter(file_name, [](CXCursor const& c){ return clang_isAttribute(clang_getCursorKind(c)); });
    static auto const vimson = extracter.extract_as_vimson(0, {});
    return vimson == "" ? NULL : vimson.c_str();
}

char const* vim_clang_extract_expressions(char const* file_name)
{
    auto extracter = libclang_vim::make_AST_extracter(file_name, [](CXCursor const& c){ return clang_isExpression(clang_getCursorKind(c)); });
    static auto const vimson = extracter.extract_as_vimson(0, {});
    return vimson == "" ? NULL : vimson.c_str();
}

char const* vim_clang_extract_preprocessings(char const* file_name)
{
    auto extracter = libclang_vim::make_AST_extracter(file_name, [](CXCursor const& c){ return clang_isPreprocessing(clang_getCursorKind(c)); });
    static auto const vimson = extracter.extract_as_vimson(0, {});
    return vimson == "" ? NULL : vimson.c_str();
}

char const* vim_clang_extract_references(char const* file_name)
{
    auto extracter = libclang_vim::make_AST_extracter(file_name, [](CXCursor const& c){ return clang_isReference(clang_getCursorKind(c)); });
    static auto const vimson = extracter.extract_as_vimson(0, {});
    return vimson == "" ? NULL : vimson.c_str();
}

char const* vim_clang_extract_statements(char const* file_name)
{
    auto extracter = libclang_vim::make_AST_extracter(file_name, [](CXCursor const& c){ return clang_isStatement(clang_getCursorKind(c)); });
    static auto const vimson = extracter.extract_as_vimson(0, {});
    return vimson == "" ? NULL : vimson.c_str();
}

char const* vim_clang_extract_translation_units(char const* file_name)
{
    auto extracter = libclang_vim::make_AST_extracter(file_name, [](CXCursor const& c){ return clang_isTranslationUnit(clang_getCursorKind(c)); });
    static auto const vimson = extracter.extract_as_vimson(0, {});
    return vimson == "" ? NULL : vimson.c_str();
}

char const* vim_clang_extract_definitions(char const* file_name)
{
    auto extracter = libclang_vim::make_AST_extracter(file_name, [](CXCursor const& c){ return clang_isCursorDefinition(c); });
    static auto const vimson = extracter.extract_as_vimson(0, {});
    return vimson == "" ? NULL : vimson.c_str();
}

char const* vim_clang_extract_virtual_member_functions(char const* file_name)
{
    auto extracter = libclang_vim::make_AST_extracter(file_name, [](CXCursor const& c){ return clang_CXXMethod_isVirtual(c); });
    static auto const vimson = extracter.extract_as_vimson(0, {});
    return vimson == "" ? NULL : vimson.c_str();
}

char const* vim_clang_extract_pure_virtual_member_functions(char const* file_name)
{
    auto extracter = libclang_vim::make_AST_extracter(file_name, [](CXCursor const& c){ return clang_CXXMethod_isPureVirtual(c); });
    static auto const vimson = extracter.extract_as_vimson(0, {});
    return vimson == "" ? NULL : vimson.c_str();
}

char const* vim_clang_extract_static_member_functions(char const* file_name)
{
    auto extracter = libclang_vim::make_AST_extracter(file_name, [](CXCursor const& c){ return clang_CXXMethod_isStatic(c); });
    static auto const vimson = extracter.extract_as_vimson(0, {});
    return vimson == "" ? NULL : vimson.c_str();
}

} // extern "C"
// }}}


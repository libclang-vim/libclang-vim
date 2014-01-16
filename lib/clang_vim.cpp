#include <cstddef>
#include <cstring>
#include <cstdio>
#include <string>
#include <fstream>
#include <vector>
#include <numeric>
#include <memory>
#include <functional>
#include <tuple>
#include <utility>
#include <algorithm>

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
                + "','line':" + std::to_string(line)
                + ",'column':" + std::to_string(column)
                + ",'offset':" + std::to_string(offset)
                + "},";
        }) + "]";
    }

public:
    std::string tokenize_as_vimson(size_t const argc, char const* args[])
    {
        CXIndex index = clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/0);

        translation_unit = clang_parseTranslationUnit(index, file_name, args, argc, NULL, 0, CXTranslationUnit_Incomplete);
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

inline std::string stringize_range(CXSourceRange const& range)
{
    if (clang_Range_isNull(range)) {
        return "";
    }
    return "'range':{'start':{" + stringize_location(clang_getRangeStart(range))
         + "},'end':{" + stringize_location(clang_getRangeEnd(range)) + "}},";
}
// }}}

// AST extracter {{{

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
        char const* file_name,
        extraction_policy const policy,
        Predicate const& predicate,
        char const* argv[] = {},
        int const argc = 0
    ) -> char const*
{
    static std::string vimson;
    vimson = "";

    typedef std::tuple<std::string&, extraction_policy const, Predicate const&> callback_data_type;
    callback_data_type callback_data{vimson, policy, predicate};

    CXIndex index = clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/0);
    CXTranslationUnit translation_unit = clang_parseTranslationUnit(index, file_name, argv, argc, NULL, 0, CXTranslationUnit_Incomplete);
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

// }}}

// Location string parser {{{
auto parse_location_string(std::string const& location_string)
    -> std::tuple<size_t, size_t, std::string>
{
    auto const end = std::end(location_string);
    auto const path_end = std::find(std::begin(location_string), end, ':');
    if (path_end == end || path_end + 1 == end) {
        return std::make_tuple(0, 0, "");
    }
    std::string file{std::begin(location_string), path_end};

    size_t line, col;
    auto const num_input = std::sscanf(std::string{path_end+1, end}.c_str(), "%zu:%zu", &line, &col);
    if (num_input != 2) {
        return std::make_tuple(0, 0, "");
    }

    return std::make_tuple(line, col, file);
}
// }}}

namespace detail {

    template<class Predicate>
    CXCursor parent_definition_cursor(CXCursor cursor, Predicate const& predicate)
    {
        while (!clang_isInvalid(clang_getCursorKind(cursor))) {
            if (predicate(cursor)){
                return cursor;
            }
            cursor = clang_getCursorSemanticParent(cursor);
        }
        return clang_getNullCursor();
    }

    bool is_class_decl(CXCursor const& cursor) {
        switch(clang_getCursorKind(cursor)) {
        case CXCursor_StructDecl:
        case CXCursor_ClassDecl:
        case CXCursor_UnionDecl:
        case CXCursor_ClassTemplate:
        case CXCursor_ClassTemplatePartialSpecialization:
            return true;
        default:
            return false;
        }
    }

    bool is_function_decl(CXCursor const& cursor) {
        switch(clang_getCursorKind(cursor)) {
        case CXCursor_FunctionDecl:
        case CXCursor_FunctionTemplate:
        case CXCursor_ConversionFunction:
        case CXCursor_CXXMethod:
        case CXCursor_ObjCInstanceMethodDecl:
        case CXCursor_ObjCClassMethodDecl:
            return true;
        default:
            return false;
        }
    }

    bool is_parameter(CXCursor const& cursor) {
        switch(clang_getCursorKind(cursor)) {
        case CXCursor_ParmDecl:
        case CXCursor_TemplateTypeParameter:
        case CXCursor_NonTypeTemplateParameter:
        case CXCursor_TemplateTemplateParameter:
            return true;
        default:
            return false;
        }
    }
} // namespace detail

template<class LocationTuple, class Predicate>
auto search_AST_upward(
        LocationTuple const& location_tuple,
        Predicate const& predicate,
        char const* argv[] = {},
        int const argc = 0
    ) -> char const*
{
    static std::string vimson;
    char const* file_name = std::get<2>(location_tuple).c_str();

    CXIndex index = clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/0);
    CXTranslationUnit translation_unit = clang_parseTranslationUnit(index, file_name, argv, argc, NULL, 0, CXTranslationUnit_Incomplete);
    if (translation_unit == NULL) {
        clang_disposeIndex(index);
        return "{}";
    }

    CXFile const file = clang_getFile(translation_unit, file_name);
    auto const location = clang_getLocation(translation_unit, file, std::get<0>(location_tuple), std::get<1>(location_tuple));
    CXCursor const result_cursor = detail::parent_definition_cursor(
            clang_getCursor(translation_unit, location),
            predicate
        );

    if (!clang_Cursor_isNull(result_cursor)) {
        auto const range = clang_getCursorExtent(result_cursor);
        vimson = "{" + stringize_range(range) + "}";
    } else {
        vimson = "{}";
    }

    clang_disposeTranslationUnit(translation_unit);
    clang_disposeIndex(index);

    return vimson.c_str();
};

} // namespace libclang_vim

// C APIs {{{
extern "C" {

char const* vim_clang_version()
{
    return clang_getCString(clang_getClangVersion());
}

char const* vim_clang_tokens(char const* file_name)
{
    libclang_vim::tokenizer tokenizer(file_name);
    static auto const vimson = tokenizer.tokenize_as_vimson(0, {});
    return vimson.c_str();
}

// API to extract all {{{
char const* vim_clang_extract_all(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::all,
                [](CXCursor const&){ return true; }
            );
}

char const* vim_clang_extract_declarations(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::all,
                [](CXCursor const& c){ return clang_isDeclaration(clang_getCursorKind(c)); }
            );
}

char const* vim_clang_extract_attributes(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::all,
                [](CXCursor const& c){ return clang_isAttribute(clang_getCursorKind(c)); }
            );
}

char const* vim_clang_extract_expressions(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::all,
                [](CXCursor const& c){ return clang_isExpression(clang_getCursorKind(c)); }
            );
}

char const* vim_clang_extract_preprocessings(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::all,
                [](CXCursor const& c){ return clang_isPreprocessing(clang_getCursorKind(c)); }
            );
}

char const* vim_clang_extract_references(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::all,
                [](CXCursor const& c){ return clang_isReference(clang_getCursorKind(c)); }
            );
}

char const* vim_clang_extract_statements(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::all,
                [](CXCursor const& c){ return clang_isStatement(clang_getCursorKind(c)); }
            );
}

char const* vim_clang_extract_translation_units(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::all,
                [](CXCursor const& c){ return clang_isTranslationUnit(clang_getCursorKind(c)); }
            );
}

char const* vim_clang_extract_definitions(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::all,
                [](CXCursor const& c){ return clang_isCursorDefinition(c); }
            );
}

char const* vim_clang_extract_virtual_member_functions(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::all,
                [](CXCursor const& c){ return clang_CXXMethod_isVirtual(c); }
            );
}

char const* vim_clang_extract_pure_virtual_member_functions(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::all,
                [](CXCursor const& c){ return clang_CXXMethod_isPureVirtual(c); }
            );
}

char const* vim_clang_extract_static_member_functions(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::all,
                [](CXCursor const& c){ return clang_CXXMethod_isStatic(c); }
            );
}
// }}}

// API to extract current file only {{{
char const* vim_clang_extract_all_current_file(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::current_file,
                [](CXCursor const&) -> bool {
                    return true;
                }
            );
}

char const* vim_clang_extract_declarations_current_file(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::current_file,
                [](CXCursor const& c){
                    return clang_isDeclaration(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_attributes_current_file(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::current_file,
                [](CXCursor const& c){
                    return clang_isAttribute(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_expressions_current_file(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::current_file,
                [](CXCursor const& c){
                    return clang_isExpression(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_preprocessings_current_file(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::current_file,
                [](CXCursor const& c){
                    return clang_isPreprocessing(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_references_current_file(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::current_file,
                [](CXCursor const& c){
                    return clang_isReference(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_statements_current_file(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::current_file,
                [](CXCursor const& c){
                    return clang_isStatement(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_translation_units_current_file(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::current_file,
                [](CXCursor const& c){
                    return clang_isTranslationUnit(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_definitions_current_file(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::current_file,
                clang_isCursorDefinition
            );
}

char const* vim_clang_extract_virtual_member_functions_current_file(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::current_file,
                clang_CXXMethod_isVirtual
            );
}

char const* vim_clang_extract_pure_virtual_member_functions_current_file(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::current_file,
                clang_CXXMethod_isPureVirtual
            );
}

char const* vim_clang_extract_static_member_functions_current_file(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::current_file,
                clang_CXXMethod_isStatic
            );
}
// }}}

// API to extract current file only {{{
char const* vim_clang_extract_all_non_system_headers(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::non_system_headers,
                [](CXCursor const&) -> bool {
                    return true;
                }
            );
}

char const* vim_clang_extract_declarations_non_system_headers(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::non_system_headers,
                [](CXCursor const& c){
                    return clang_isDeclaration(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_attributes_non_system_headers(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::non_system_headers,
                [](CXCursor const& c){
                    return clang_isAttribute(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_expressions_non_system_headers(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::non_system_headers,
                [](CXCursor const& c){
                    return clang_isExpression(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_preprocessings_non_system_headers(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::non_system_headers,
                [](CXCursor const& c){
                    return clang_isPreprocessing(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_references_non_system_headers(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::non_system_headers,
                [](CXCursor const& c){
                    return clang_isReference(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_statements_non_system_headers(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::non_system_headers,
                [](CXCursor const& c){
                    return clang_isStatement(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_translation_units_non_system_headers(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::non_system_headers,
                [](CXCursor const& c){
                    return clang_isTranslationUnit(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_extract_definitions_non_system_headers(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::non_system_headers,
                clang_isCursorDefinition
            );
}

char const* vim_clang_extract_virtual_member_functions_non_system_headers(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::non_system_headers,
                clang_CXXMethod_isVirtual
            );
}

char const* vim_clang_extract_pure_virtual_member_functions_non_system_headers(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::non_system_headers,
                clang_CXXMethod_isPureVirtual
            );
}

char const* vim_clang_extract_static_member_functions_non_system_headers(char const* file_name)
{
    return libclang_vim::extract_AST_nodes(
                file_name,
                libclang_vim::extraction_policy::non_system_headers,
                clang_CXXMethod_isStatic
            );
}
// }}}

// API to get information of specific location {{{
char const* vim_clang_get_location_information(char const* location_string)
{
    auto location_info = libclang_vim::parse_location_string(location_string);
    char const* file_name = std::get<2>(location_info).c_str();
    CXIndex index = clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/0);
    CXTranslationUnit translation_unit = clang_parseTranslationUnit(index, file_name, {}, 0, NULL, 0, CXTranslationUnit_Incomplete);
    if (translation_unit == NULL) {
        clang_disposeIndex(index);
        return "{}";
    }

    CXFile const file = clang_getFile(translation_unit, file_name);
    auto const location = clang_getLocation(translation_unit, file, std::get<0>(location_info), std::get<1>(location_info));
    CXCursor const cursor = clang_getCursor(translation_unit, location);
    static std::string result;
    result = "{" + libclang_vim::stringize_cursor(cursor, clang_getCursorSemanticParent(cursor)) + "}";

    clang_disposeTranslationUnit(translation_unit);
    clang_disposeIndex(index);

    return result.c_str();
}
// }}}

// API to get extent of identifier at specific location {{{
char const* vim_clang_get_extent_of_node_at_specific_location(char const* location_string)
{
    auto location_info = libclang_vim::parse_location_string(location_string);
    char const* file_name = std::get<2>(location_info).c_str();
    CXIndex index = clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/0);
    CXTranslationUnit translation_unit = clang_parseTranslationUnit(index, file_name, {}, 0, NULL, 0, CXTranslationUnit_Incomplete);
    if (translation_unit == NULL) {
        clang_disposeIndex(index);
        return "{}";
    }

    CXFile const file = clang_getFile(translation_unit, file_name);
    auto const location = clang_getLocation(translation_unit, file, std::get<0>(location_info), std::get<1>(location_info));
    CXCursor const cursor = clang_getCursor(translation_unit, location);
    auto const range = clang_getCursorExtent(cursor);
    static std::string result;
    result = "{" + libclang_vim::stringize_range(range) + "}";

    clang_disposeTranslationUnit(translation_unit);
    clang_disposeIndex(index);

    return result.c_str();
}

char const* vim_clang_get_inner_definition_extent_at_specific_location(char const* location_string)
{
    auto const parsed_location = libclang_vim::parse_location_string(location_string);
    return libclang_vim::search_AST_upward(
                parsed_location,
                clang_isCursorDefinition
            );
}

char const* vim_clang_get_expression_extent_at_specific_location(char const* location_string)
{
    auto const parsed_location = libclang_vim::parse_location_string(location_string);
    return libclang_vim::search_AST_upward(
                parsed_location,
                [](CXCursor const& c){
                    return clang_isExpression(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_get_statement_extent_at_specific_location(char const* location_string)
{
    auto const parsed_location = libclang_vim::parse_location_string(location_string);
    return libclang_vim::search_AST_upward(
                parsed_location,
                [](CXCursor const& c){
                    return clang_isStatement(clang_getCursorKind(c));
                }
            );
}

char const* vim_clang_get_class_extent_at_specific_location(char const* location_string)
{
    auto const parsed_location = libclang_vim::parse_location_string(location_string);
    return libclang_vim::search_AST_upward(
                parsed_location,
                libclang_vim::detail::is_class_decl
            );
}

char const* vim_clang_get_function_extent_at_specific_location(char const* location_string)
{
    auto const parsed_location = libclang_vim::parse_location_string(location_string);
    return libclang_vim::search_AST_upward(
                parsed_location,
                libclang_vim::detail::is_function_decl
            );
}

char const* vim_clang_get_parameter_extent_at_specific_location(char const* location_string)
{
    auto const parsed_location = libclang_vim::parse_location_string(location_string);
    return libclang_vim::search_AST_upward(
                parsed_location,
                libclang_vim::detail::is_parameter
            );
}
// }}}

} // extern "C"
// }}}


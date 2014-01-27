#if !defined LIBCLANG_VIM_HELPERS_HPP_INCLUDED
#define      LIBCLANG_VIM_HELPERS_HPP_INCLUDED

#include <cstring>
#include <cstddef>
#include <string>
#include <memory>
#include <fstream>
#include <algorithm>

#include <clang-c/Index.h>

namespace libclang_vim {

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
    case CXCursor_Constructor:
    case CXCursor_Destructor:
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

// Location string parser
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

template<class LocationTuple, class Predicate>
auto at_specific_location(
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
    CXCursor const cursor = clang_getCursor(translation_unit, location);

    vimson = predicate(cursor);

    clang_disposeTranslationUnit(translation_unit);
    clang_disposeIndex(index);

    return vimson.c_str();
}

namespace detail {

template<class DataType>
CXChildVisitResult search_kind_visitor(CXCursor cursor, CXCursor, CXClientData data)
{
    auto const kind = clang_getCursorKind(cursor);
    if ((reinterpret_cast<DataType *>(data)->second(kind))) {
        (reinterpret_cast<DataType *>(data))->first = cursor;
        return CXChildVisit_Break;
    }

    clang_visitChildren(cursor, search_kind_visitor<DataType>, data);
    return CXChildVisit_Continue;
}

} // namespace detail

template<class Predicate>
CXCursor search_kind(CXCursor const& cursor, Predicate const& predicate)
{
    auto const kind = clang_getCursorKind(cursor);
    if (predicate(kind)) {
        return cursor;
    }

    auto kind_visitor_data = std::make_pair(clang_getNullCursor(), predicate);
    clang_visitChildren(cursor, detail::search_kind_visitor<decltype(kind_visitor_data)>, &kind_visitor_data);
    return kind_visitor_data.first;
}

} // namespace libclang_vim

#endif    // LIBCLANG_VIM_HELPERS_HPP_INCLUDED

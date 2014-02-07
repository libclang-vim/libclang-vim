#if !defined LIBCLANG_VIM_HELPERS_HPP_INCLUDED
#define      LIBCLANG_VIM_HELPERS_HPP_INCLUDED

#include <cstring>
#include <cstddef>
#include <string>
#include <memory>
#include <fstream>
#include <algorithm>
#include <vector>
#include <sstream>
#include <iterator>
#include <utility>

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

bool is_class_decl_kind(CXCursorKind const& kind)
{
    switch(kind) {
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

bool is_class_decl(CXCursor const& cursor)
{
    return is_class_decl_kind(clang_getCursorKind(cursor));
}

bool is_function_decl_kind(CXCursorKind const& kind)
{
    switch(kind) {
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

bool is_function_decl(CXCursor const& cursor)
{
    return is_function_decl_kind(clang_getCursorKind(cursor));
}

bool is_parameter_kind(CXCursorKind const& kind)
{
    switch(kind) {
    case CXCursor_ParmDecl:
    case CXCursor_TemplateTypeParameter:
    case CXCursor_NonTypeTemplateParameter:
    case CXCursor_TemplateTemplateParameter:
        return true;
    default:
        return false;
    }
}

bool is_parameter(CXCursor const& cursor)
{
    return is_parameter_kind(clang_getCursorKind(cursor));
}

using args_type = std::vector<std::string>;

namespace detail {

    args_type parse_compiler_args(std::string const& s)
    {
        using iterator = std::istream_iterator<std::string>;
        args_type result;
        std::istringstream iss(s);
        std::copy(iterator(iss), iterator{}, std::back_inserter(result));
        return result;
    }

} // namespace detail

// Parse "file:args"
auto parse_default_args(std::string const& args_string)
    -> std::pair<std::string, args_type>
{
    auto const end = std::end(args_string);
    auto const path_end = std::find(std::begin(args_string), end, ':');
    if (path_end == end) {
        return {"", args_type{}};
    };
    std::string const file{std::begin(args_string), path_end};
    if (path_end+1 == end) {
        return {file, {}};
    } else {
        return {file, detail::parse_compiler_args({path_end+1, end})};
    }
}

// Parse "file:args:line:col"
auto parse_args_with_location(std::string const& args_string)
    -> std::tuple<std::string, args_type, size_t, size_t>
{
    auto const end = std::end(args_string);

    auto second_colon = std::find(std::begin(args_string), end, ':');
    if (second_colon == end || second_colon + 1 == end) {
        return std::make_tuple("", args_type{}, 0, 0);
    }
    second_colon = std::find(second_colon + 1, end, ':');
    if (second_colon == end || second_colon + 1 == end) {
        return std::make_tuple("", args_type{}, 0, 0);
    }

    auto const default_args = parse_default_args({std::begin(args_string), second_colon});
    if (default_args.first == "") {
        return std::make_tuple("", args_type{}, 0, 0);
    }

    size_t line, col;
    auto const num_input = std::sscanf(std::string{second_colon+1, end}.c_str(), "%zu:%zu", &line, &col);
    if (num_input != 2) {
        return std::make_tuple("", args_type{}, 0, 0);
    }

    return std::make_tuple(default_args.first, default_args.second, line, col);
}

inline std::vector<char const *> get_args_ptrs(args_type const& args)
{
    std::vector<char const*> args_ptrs{args.size()};
    std::transform(std::begin(args), std::end(args), std::begin(args_ptrs), [](std::string const& s){ return s.c_str(); });
    return args_ptrs;
}

template<class LocationTuple, class Predicate>
auto at_specific_location(
        LocationTuple const& location_tuple,
        Predicate const& predicate
    ) -> char const*
{
    static std::string vimson;
    char const* file_name = std::get<0>(location_tuple).c_str();
    auto const args_ptrs = get_args_ptrs(std::get<1>(location_tuple));

    CXIndex index = clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/0);
    CXTranslationUnit translation_unit = clang_parseTranslationUnit(index, file_name, args_ptrs.data(), args_ptrs.size(), NULL, 0, CXTranslationUnit_Incomplete);
    if (translation_unit == NULL) {
        clang_disposeIndex(index);
        return "{}";
    }

    CXFile const file = clang_getFile(translation_unit, file_name);
    auto const location = clang_getLocation(translation_unit, file, std::get<2>(location_tuple), std::get<3>(location_tuple));
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

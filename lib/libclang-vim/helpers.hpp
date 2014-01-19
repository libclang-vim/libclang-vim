#if !defined LIBCLANG_VIM_HELPERS_HPP_INCLUDED
#define      LIBCLANG_VIM_HELPERS_HPP_INCLUDED

#include <cstring>
#include <cstddef>
#include <string>
#include <memory>
#include <fstream>

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

} // namespace libclang_vim

#endif    // LIBCLANG_VIM_HELPERS_HPP_INCLUDED

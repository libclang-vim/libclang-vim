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

inline bool is_null_location(const CXSourceLocation& location)
{
    return clang_equalLocations(location, clang_getNullLocation());
}

/// Class to avoid the need to call clang_disposeIndex() manually.
class cxindex_ptr
{
    CXIndex _index;

public:
    cxindex_ptr(CXIndex index)
        : _index(index)
    {
    }

    operator const CXIndex&() const
    {
        return _index;
    }

    operator bool() const
    {
        return _index != nullptr;
    }

    ~cxindex_ptr()
    {
        if (_index)
            clang_disposeIndex(_index);
    }
};

/// Class to avoid the need to call clang_disposeTranslationUnit() manually.
class cxtranslation_unit_ptr
{
    CXTranslationUnit _unit;

public:
    cxtranslation_unit_ptr(CXTranslationUnit unit)
        : _unit(unit)
    {
    }

    operator const CXTranslationUnit&() const
    {
        return _unit;
    }

    operator bool() const
    {
        return _unit != nullptr;
    }

    ~cxtranslation_unit_ptr()
    {
        if (_unit)
            clang_disposeTranslationUnit(_unit);
    }
};

/// Class to avoid the need to call clang_disposeString() manually.
class cxstring_ptr
{
    CXString _string;

public:
    cxstring_ptr(CXString string)
        : _string(string)
    {
    }

    operator const CXString&() const
    {
        return _string;
    }

    ~cxstring_ptr()
    {
        clang_disposeString(_string);
    }
};

inline char const* to_c_str(const cxstring_ptr& string)
{
    return clang_getCString(string);
}

inline std::string stringize_key_value(char const* key_name, cxstring_ptr const& p)
{
    auto const* cstring = clang_getCString(p);
    if (!cstring || std::strcmp(cstring, "") == 0) {
        return "";
    } else {
        return "'" + std::string{key_name} + "':'" + cstring + "',";
    }
}

inline std::string stringize_key_value(char const* key_name, std::string const& s)
{
    if (s.empty()) {
        return "";
    } else {
        return "'" + (key_name + ("':'" + s + "',"));
    }
}

inline bool is_class_decl_kind(CXCursorKind const& kind)
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

inline bool is_class_decl(CXCursor const& cursor)
{
    return is_class_decl_kind(clang_getCursorKind(cursor));
}

inline bool is_function_decl_kind(CXCursorKind const& kind)
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

inline bool is_function_decl(CXCursor const& cursor)
{
    return is_function_decl_kind(clang_getCursorKind(cursor));
}

inline bool is_parameter_kind(CXCursorKind const& kind)
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

inline bool is_parameter(CXCursor const& cursor)
{
    return is_parameter_kind(clang_getCursorKind(cursor));
}

using args_type = std::vector<std::string>;

namespace detail {

    inline args_type parse_compiler_args(std::string const& s)
    {
        using iterator = std::istream_iterator<std::string>;
        args_type result;
        std::istringstream iss(s);
        std::copy(iterator(iss), iterator{}, std::back_inserter(result));
        return result;
    }

} // namespace detail

// Parse "file:args"
inline auto parse_default_args(std::string const& args_string)
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

/// Stores compiler arguments with location.
class location_tuple
{
public:
    std::string file;
    /// Contents of the unsaved buffer of file.
    std::vector<char> unsaved_file;
    args_type args;
    size_t line;
    size_t col;

    location_tuple()
        : line(0),
        col(0)
    {
    }
};

/// Creates a CXUnsavedFile array, suitable for clang_parseTranslationUnit().
inline std::vector<CXUnsavedFile> create_unsaved_files(const location_tuple& location_info)
{
    std::vector<CXUnsavedFile> unsaved_files;
    if (!location_info.unsaved_file.empty())
    {
        CXUnsavedFile unsaved_file;
        unsaved_file.Filename = location_info.file.c_str();
        unsaved_file.Contents = location_info.unsaved_file.data();
        unsaved_file.Length = location_info.unsaved_file.size();
        unsaved_files.push_back(unsaved_file);
    }
    return unsaved_files;
}

/// Parse "file:args:line:col".
location_tuple parse_args_with_location(const std::string& args_string);

std::vector<const char*> get_args_ptrs(const args_type& args);

const char* at_specific_location(const location_tuple& location_tuple, const std::function<std::string(CXCursor const&)>& predicate);

CXCursor search_kind(const CXCursor& cursor, const std::function<bool(const CXCursorKind&)>& predicate);

} // namespace libclang_vim

#endif    // LIBCLANG_VIM_HELPERS_HPP_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

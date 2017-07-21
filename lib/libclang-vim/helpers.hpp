#if !defined LIBCLANG_VIM_HELPERS_HPP_INCLUDED
#define LIBCLANG_VIM_HELPERS_HPP_INCLUDED

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <functional>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <clang-c/Index.h>

namespace libclang_vim {

using std::size_t;

size_t get_file_size(const char* filename);

bool is_null_location(const CXSourceLocation& location);

/// Class to avoid the need to call clang_disposeIndex() manually.
class cxindex_ptr {
    CXIndex _index;

  public:
    cxindex_ptr(CXIndex index);

    operator const CXIndex&() const;

    operator bool() const;

    ~cxindex_ptr();
};

/// Class to avoid the need to call clang_disposeTranslationUnit() manually.
class cxtranslation_unit_ptr {
    CXTranslationUnit _unit;

  public:
    cxtranslation_unit_ptr(CXTranslationUnit unit);

    operator const CXTranslationUnit&() const;

    operator bool() const;

    ~cxtranslation_unit_ptr();
};

/// Class to avoid the need to call clang_disposeString() manually.
class cxstring_ptr {
    CXString _string;

  public:
    cxstring_ptr(CXString string);

    operator const CXString&() const;

    ~cxstring_ptr();
};

const char* to_c_str(const cxstring_ptr& string);

std::string stringize_key_value(const char* key_name, const cxstring_ptr& p);

std::string stringize_key_value(const char* key_name, const std::string& s);

bool is_class_decl_kind(const CXCursorKind& kind);

bool is_class_decl(const CXCursor& cursor);

bool is_function_decl_kind(const CXCursorKind& kind);

bool is_function_decl(const CXCursor& cursor);

bool is_parameter_kind(const CXCursorKind& kind);

bool is_parameter(const CXCursor& cursor);

using args_type = std::vector<std::string>;

/// Stores compiler arguments with location.
class location_tuple {
  public:
    std::string file;
    /// Contents of the unsaved buffer of file.
    std::vector<char> unsaved_file;
    args_type args;
    size_t line = 0;
    size_t col = 0;

    location_tuple();
};

/// Parse "file:args".
location_tuple parse_default_args(const std::string& args_string);

/// Creates a CXUnsavedFile array, suitable for clang_parseTranslationUnit().
std::vector<CXUnsavedFile>
create_unsaved_files(const location_tuple& location_info);

/// Set info.unsaved_file if info.file is in "real filename#temp file" syntax.
void extract_unsaved_file(libclang_vim::location_tuple& info);

/// Parse "file:args:line:col".
location_tuple parse_args_with_location(const std::string& args_string);

std::vector<const char*> get_args_ptrs(const args_type& args);

const char* at_specific_location(
    const location_tuple& location_tuple,
    const std::function<std::string(CXCursor const&)>& predicate);

CXCursor search_kind(const CXCursor& cursor,
                     const std::function<bool(const CXCursorKind&)>& predicate);

} // namespace libclang_vim

#endif // LIBCLANG_VIM_HELPERS_HPP_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

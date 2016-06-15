#include "helpers.hpp"

namespace {

using DataType =
    std::pair<CXCursor, const std::function<bool(const CXCursorKind&)>&>;

CXChildVisitResult search_kind_visitor(CXCursor cursor, CXCursor,
                                       CXClientData data) {
    auto const kind = clang_getCursorKind(cursor);
    if ((reinterpret_cast<DataType*>(data)->second(kind))) {
        (reinterpret_cast<DataType*>(data))->first = cursor;
        return CXChildVisit_Break;
    }

    clang_visitChildren(cursor, search_kind_visitor, data);
    return CXChildVisit_Continue;
}

libclang_vim::args_type parse_compiler_args(const std::string& s) {
    using iterator = std::istream_iterator<std::string>;
    libclang_vim::args_type result;
    std::istringstream iss(s);
    std::copy(iterator(iss), iterator{}, std::back_inserter(result));
    return result;
}
}

size_t libclang_vim::get_file_size(const char* filename) {
    std::ifstream input(filename);
    return input.seekg(0, std::ios::end).tellg();
}

bool libclang_vim::is_null_location(const CXSourceLocation& location) {
    return clang_equalLocations(location, clang_getNullLocation());
}

libclang_vim::cxindex_ptr::cxindex_ptr(CXIndex index) : _index(index) {}

libclang_vim::cxindex_ptr::operator const CXIndex&() const { return _index; }

libclang_vim::cxindex_ptr::operator bool() const { return _index != nullptr; }

libclang_vim::cxindex_ptr::~cxindex_ptr() {
    if (_index)
        clang_disposeIndex(_index);
}

libclang_vim::cxtranslation_unit_ptr::cxtranslation_unit_ptr(
    CXTranslationUnit unit)
    : _unit(unit) {}

libclang_vim::cxtranslation_unit_ptr::
operator const CXTranslationUnit&() const {
    return _unit;
}

libclang_vim::cxtranslation_unit_ptr::operator bool() const {
    return _unit != nullptr;
}

libclang_vim::cxtranslation_unit_ptr::~cxtranslation_unit_ptr() {
    if (_unit)
        clang_disposeTranslationUnit(_unit);
}

libclang_vim::cxstring_ptr::cxstring_ptr(CXString string) : _string(string) {}

libclang_vim::cxstring_ptr::operator const CXString&() const { return _string; }

libclang_vim::cxstring_ptr::~cxstring_ptr() { clang_disposeString(_string); }

const char* libclang_vim::to_c_str(const libclang_vim::cxstring_ptr& string) {
    return clang_getCString(string);
}

std::string libclang_vim::stringize_key_value(const char* key_name,
                                              const cxstring_ptr& p) {
    const auto* cstring = clang_getCString(p);
    if (!cstring || std::strcmp(cstring, "") == 0)
        return "";
    else
        return "'" + std::string{key_name} + "':'" + cstring + "',";
}

std::string libclang_vim::stringize_key_value(const char* key_name,
                                              const std::string& s) {
    if (s.empty())
        return "";
    else
        return "'" + (key_name + ("':'" + s + "',"));
}

bool libclang_vim::is_class_decl_kind(const CXCursorKind& kind) {
    switch (kind) {
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

bool libclang_vim::is_class_decl(const CXCursor& cursor) {
    return is_class_decl_kind(clang_getCursorKind(cursor));
}

bool libclang_vim::is_function_decl_kind(const CXCursorKind& kind) {
    switch (kind) {
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

bool libclang_vim::is_function_decl(const CXCursor& cursor) {
    return is_function_decl_kind(clang_getCursorKind(cursor));
}

bool libclang_vim::is_parameter_kind(const CXCursorKind& kind) {
    switch (kind) {
    case CXCursor_ParmDecl:
    case CXCursor_TemplateTypeParameter:
    case CXCursor_NonTypeTemplateParameter:
    case CXCursor_TemplateTemplateParameter:
        return true;
    default:
        return false;
    }
}

bool libclang_vim::is_parameter(const CXCursor& cursor) {
    return is_parameter_kind(clang_getCursorKind(cursor));
}

libclang_vim::location_tuple
libclang_vim::parse_default_args(const std::string& args_string) {
    location_tuple info;
    const auto end = std::end(args_string);
    const auto path_end = std::find(std::begin(args_string), end, ':');
    if (path_end == end)
        return info;

    const std::string file{std::begin(args_string), path_end};
    info.file = file;
    extract_unsaved_file(info);
    if (path_end + 1 == end)
        return info;
    else {
        info.args = parse_compiler_args({path_end + 1, end});
        return info;
    }
}

libclang_vim::location_tuple::location_tuple() : line(0), col(0) {}

std::vector<CXUnsavedFile>
libclang_vim::create_unsaved_files(const location_tuple& location_info) {
    std::vector<CXUnsavedFile> unsaved_files;
    if (!location_info.unsaved_file.empty()) {
        CXUnsavedFile unsaved_file;
        unsaved_file.Filename = location_info.file.c_str();
        unsaved_file.Contents = location_info.unsaved_file.data();
        unsaved_file.Length = location_info.unsaved_file.size();
        unsaved_files.push_back(unsaved_file);
    }
    return unsaved_files;
}

void libclang_vim::extract_unsaved_file(libclang_vim::location_tuple& info) {
    // Recognize "real filename#temp file" syntax, in which case assume the
    // later is the unsaved version of the previous.
    std::size_t pos = info.file.find('#');
    if (pos != std::string::npos) {
        std::string unsaved_path = info.file.substr(pos + 1);
        info.file = info.file.substr(0, pos);
        std::ifstream unsaved_stream(unsaved_path,
                                     std::ios::in | std::ios::binary);
        info.unsaved_file =
            std::vector<char>((std::istreambuf_iterator<char>(unsaved_stream)),
                              std::istreambuf_iterator<char>());
    }
}

libclang_vim::location_tuple
libclang_vim::parse_args_with_location(const std::string& args_string) {
    auto const end = std::end(args_string);

    auto second_colon = std::find(std::begin(args_string), end, ':');
    if (second_colon == end || second_colon + 1 == end) {
        return location_tuple();
    }
    second_colon = std::find(second_colon + 1, end, ':');
    if (second_colon == end || second_colon + 1 == end) {
        return location_tuple();
    }

    auto const default_args =
        parse_default_args({std::begin(args_string), second_colon});
    if (default_args.file == "") {
        return location_tuple();
    }

    size_t line, col;
    auto const num_input = std::sscanf(
        std::string{second_colon + 1, end}.c_str(), "%zu:%zu", &line, &col);
    if (num_input != 2) {
        return location_tuple();
    }

    location_tuple ret;
    ret.file = default_args.file;
    ret.unsaved_file = default_args.unsaved_file;
    ret.args = default_args.args;
    ret.line = line;
    ret.col = col;
    return ret;
}

std::vector<const char*> libclang_vim::get_args_ptrs(const args_type& args) {
    std::vector<const char*> args_ptrs{args.size()};
    std::transform(std::begin(args), std::end(args), std::begin(args_ptrs),
                   [](const std::string& s) { return s.c_str(); });
    return args_ptrs;
}

const char* libclang_vim::at_specific_location(
    const location_tuple& location_tuple,
    const std::function<std::string(CXCursor const&)>& predicate) {
    static std::string vimson;
    char const* file_name = location_tuple.file.c_str();
    auto const args_ptrs = get_args_ptrs(location_tuple.args);

    cxindex_ptr index =
        clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/ 0);
    cxtranslation_unit_ptr translation_unit(clang_parseTranslationUnit(
        index, file_name, args_ptrs.data(), args_ptrs.size(), nullptr, 0,
        CXTranslationUnit_Incomplete));
    if (!translation_unit)
        return "{}";

    CXFile file = clang_getFile(translation_unit, file_name);
    auto const location = clang_getLocation(
        translation_unit, file, location_tuple.line, location_tuple.col);
    CXCursor const cursor = clang_getCursor(translation_unit, location);

    vimson = predicate(cursor);

    return vimson.c_str();
}

CXCursor libclang_vim::search_kind(
    const CXCursor& cursor,
    const std::function<bool(const CXCursorKind&)>& predicate) {
    const auto kind = clang_getCursorKind(cursor);
    if (predicate(kind)) {
        return cursor;
    }

    auto kind_visitor_data = std::make_pair(clang_getNullCursor(), predicate);
    clang_visitChildren(cursor, search_kind_visitor, &kind_visitor_data);
    return kind_visitor_data.first;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

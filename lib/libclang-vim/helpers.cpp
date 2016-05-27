#include "helpers.hpp"

namespace
{

using DataType = std::pair<CXCursor, const std::function<bool(const CXCursorKind&)>&>;

CXChildVisitResult search_kind_visitor(CXCursor cursor, CXCursor, CXClientData data)
{
    auto const kind = clang_getCursorKind(cursor);
    if ((reinterpret_cast<DataType *>(data)->second(kind)))
    {
        (reinterpret_cast<DataType *>(data))->first = cursor;
        return CXChildVisit_Break;
    }

    clang_visitChildren(cursor, search_kind_visitor, data);
    return CXChildVisit_Continue;
}

}

libclang_vim::location_tuple libclang_vim::parse_args_with_location(const std::string& args_string)
{
    auto const end = std::end(args_string);

    auto second_colon = std::find(std::begin(args_string), end, ':');
    if (second_colon == end || second_colon + 1 == end)
    {
        return location_tuple();
    }
    second_colon = std::find(second_colon + 1, end, ':');
    if (second_colon == end || second_colon + 1 == end)
    {
        return location_tuple();
    }

    auto const default_args = parse_default_args({std::begin(args_string), second_colon});
    if (default_args.first == "")
    {
        return location_tuple();
    }

    size_t line, col;
    auto const num_input = std::sscanf(std::string{second_colon+1, end}.c_str(), "%zu:%zu", &line, &col);
    if (num_input != 2)
    {
        return location_tuple();
    }

    location_tuple ret;
    ret.file = default_args.first;

    // Recognize "real filename#temp file" syntax, in which case assume the later is the unsaved version of the previous.
    std::size_t pos = ret.file.find('#');
    if (pos != std::string::npos)
    {
        std::string unsaved_path = ret.file.substr(pos + 1);
        ret.file = ret.file.substr(0, pos);
        std::ifstream unsaved_stream(unsaved_path, std::ios::in | std::ios::binary);
        ret.unsaved_file = std::vector<char>((std::istreambuf_iterator<char>(unsaved_stream)), std::istreambuf_iterator<char>());
    }

    ret.args = default_args.second;
    ret.line = line;
    ret.col = col;
    return ret;
}

std::vector<const char*> libclang_vim::get_args_ptrs(const args_type& args)
{
    std::vector<const char*> args_ptrs{args.size()};
    std::transform(std::begin(args), std::end(args), std::begin(args_ptrs), [](const std::string& s){ return s.c_str(); });
    return args_ptrs;
}

const char* libclang_vim::at_specific_location(const location_tuple& location_tuple, const std::function<std::string(CXCursor const&)>& predicate)
{
    static std::string vimson;
    char const* file_name = location_tuple.file.c_str();
    auto const args_ptrs = get_args_ptrs(location_tuple.args);

    cxindex_ptr index = clang_createIndex(/*excludeDeclsFromPCH*/ 1, /*displayDiagnostics*/0);
    cxtranslation_unit_ptr translation_unit(clang_parseTranslationUnit(index, file_name, args_ptrs.data(), args_ptrs.size(), nullptr, 0, CXTranslationUnit_Incomplete));
    if (!translation_unit)
        return "{}";

    CXFile const file = clang_getFile(translation_unit, file_name);
    auto const location = clang_getLocation(translation_unit, file, location_tuple.line, location_tuple.col);
    CXCursor const cursor = clang_getCursor(translation_unit, location);

    vimson = predicate(cursor);

    return vimson.c_str();
}

CXCursor libclang_vim::search_kind(const CXCursor& cursor, const std::function<bool(const CXCursorKind&)>& predicate)
{
    const auto kind = clang_getCursorKind(cursor);
    if (predicate(kind))
    {
        return cursor;
    }

    auto kind_visitor_data = std::make_pair(clang_getNullCursor(), predicate);
    clang_visitChildren(cursor, search_kind_visitor, &kind_visitor_data);
    return kind_visitor_data.first;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

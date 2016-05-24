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

#if !defined LIBCLANG_VIM_STRINGIZERS_HPP_INCLUDED
#define      LIBCLANG_VIM_STRINGIZERS_HPP_INCLUDED

#include <string>

#include <clang-c/Index.h>

#include "helpers.hpp"

namespace libclang_vim {

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

inline std::string stringize_type(CXType const& type)
{
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

    return "'line':" + std::to_string(line)
        + ",'column':" + std::to_string(column)
        + ",'offset':" + std::to_string(offset) + ','
        + stringize_key_value("file", file_name);
}

inline std::string stringize_cursor_location(CXCursor const& cursor)
{
    CXSourceLocation const location = clang_getCursorLocation(cursor);
    return stringize_location(location);
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
        + stringize_type(clang_getCursorType(cursor))
        + stringize_linkage(cursor)
        + stringize_parent(cursor, parent)
        + stringize_cursor_location(cursor)
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

inline std::string stringize_extent(CXCursor const& cursor)
{
    auto const r = clang_getCursorExtent(cursor);
    if (clang_Range_isNull(r)) {
        return "";
    } else {
        return "'start':{" + stringize_location(clang_getRangeStart(r)) + "},'end':{" + stringize_location(clang_getRangeEnd(r)) + "}";
    }
}

} // namespace libclang_vim

#endif    // LIBCLANG_VIM_STRINGIZERS_HPP_INCLUDED

#include "deduction.hpp"

#include <clang-c/CXCompilationDatabase.h>

namespace
{

/// Look up compilation arguments for a file from a database in one of its parent directories.
libclang_vim::args_type parse_compilation_database(const std::string& file)
{
    libclang_vim::args_type ret;

    std::size_t found = file.find_last_of("/\\");
    std::string directory = file.substr(0, found);
    while (true)
    {
        std::string json = directory + file.substr(found, 1) + "compile_commands.json";
        std::ifstream stream(json.c_str());
        if (stream.good())
            break;

        found = directory.find_last_of("/\\");
        if (found == std::string::npos)
            break;

        directory = directory.substr(0, found);
    }

    if (directory.empty())
    {
        // Our default when no JSON was found.
        ret.push_back("-std=c++1y");

        return ret;
    }

    CXCompilationDatabase_Error error;
    CXCompilationDatabase database = clang_CompilationDatabase_fromDirectory(directory.c_str(), &error);
    if (error == CXCompilationDatabase_NoError)
    {
        CXCompileCommands commands = clang_CompilationDatabase_getCompileCommands(database, file.c_str());
        unsigned commandsSize = clang_CompileCommands_getSize(commands);
        if (commandsSize >= 1)
        {
            CXCompileCommand command = clang_CompileCommands_getCommand(commands, 0);
            unsigned args = clang_CompileCommand_getNumArgs(command);
            for (unsigned i = 0; i < args; ++i)
            {
                libclang_vim::cxstring_ptr arg = clang_CompileCommand_getArg(command, i);
                if (file != clang_getCString(arg))
                    ret.push_back(clang_getCString(arg));
            }
        }
        clang_CompileCommands_dispose(commands);
    }
    clang_CompilationDatabase_dispose(database);

    return ret;
}

}

const char* libclang_vim::get_compile_commands(const std::string& file)
{
    static std::string vimson;

    // Write the header.
    std::stringstream ss;
    ss << "{'commands':'";

    args_type args = parse_compilation_database(file);
    for (std::size_t i = 0; i < args.size(); ++i)
    {
        if (i)
            ss << " ";
        ss << args[i];
    }

    // Write the footer.
    ss << "'}";
    vimson = ss.str();
    return vimson.c_str();
}

const char* libclang_vim::get_current_function_at(const location_tuple& location_info)
{
    static std::string vimson;

    // Write the header.
    std::stringstream ss;
    ss << "{'name':'";

    // Write the actual name.
    cxindex_ptr index = clang_createIndex(/*excludeDeclsFromPCH=*/1, /*displayDiagnostics=*/0);

    std::string file_name = location_info.file;
    std::vector<const char*> args_ptrs = get_args_ptrs(location_info.args);
    cxtranslation_unit_ptr translation_unit(clang_parseTranslationUnit(index, file_name.c_str(), args_ptrs.data(), args_ptrs.size(), nullptr, 0, CXTranslationUnit_Incomplete));
    if (!translation_unit)
        return "{}";

    const CXFile file = clang_getFile(translation_unit, file_name.c_str());
    unsigned line = location_info.line;
    unsigned column = location_info.col;

    CXCursor cursor;
    CXCursorKind kind;
    while (true)
    {
        CXSourceLocation location = clang_getLocation(translation_unit, file, line, column);
        cursor = clang_getCursor(translation_unit, location);
        kind = clang_getCursorKind(cursor);
        if (clang_getCursorKind(clang_getCursorSemanticParent(cursor)) != CXCursor_InvalidFile || column <= 1)
            break;

        // This happens with e.g. CXCursor_TypeRef, work it around by going
        // back till we get a sane parent, if we can.
        --column;
    }

    while (true)
    {
        if (is_function_decl_kind(kind) || kind == CXCursor_TranslationUnit)
            break;
        cursor = clang_getCursorSemanticParent(cursor);
        kind = clang_getCursorKind(cursor);
    }

    if (kind != CXCursor_TranslationUnit)
    {
        std::stack<std::string> stack;
        while (true)
        {
            cxstring_ptr aString = clang_getCursorSpelling(cursor);
            stack.push(clang_getCString(aString));

            cursor = clang_getCursorSemanticParent(cursor);
            if (clang_getCursorKind(cursor) == CXCursor_TranslationUnit)
                break;
        }
        bool first = true;
        while (!stack.empty())
        {
            if (first)
                first = false;
            else
                ss << "::";
            ss << stack.top();
            stack.pop();
        }
    }

    // Write the footer.
    ss << "'}";
    vimson = ss.str();
    return vimson.c_str();
}

const char* libclang_vim::get_comment_at(const location_tuple& location_info)
{
    static std::string vimson;

    // Write the header.
    std::stringstream ss;
    ss << "{'brief':'";

    // Write the actual comment.
    cxindex_ptr index = clang_createIndex(/*excludeDeclsFromPCH=*/1, /*displayDiagnostics=*/0);

    std::string file_name = location_info.file;
    std::vector<const char*> args_ptrs = get_args_ptrs(location_info.args);
    cxtranslation_unit_ptr translation_unit(clang_parseTranslationUnit(index, file_name.c_str(), args_ptrs.data(), args_ptrs.size(), nullptr, 0, CXTranslationUnit_Incomplete));
    if (!translation_unit)
        return "{}";

    const CXFile file = clang_getFile(translation_unit, file_name.c_str());
    int line = location_info.line;
    int column = location_info.col;
    CXSourceLocation source_location = clang_getLocation(translation_unit, file, line, column);
    CXCursor cursor = clang_getCursor(translation_unit, source_location);
    if (clang_Cursor_isNull(cursor) || clang_isInvalid(clang_getCursorKind(cursor)))
        return "{}";

    CXCursor referenced_cursor = clang_getCursorReferenced(cursor);
    if (!clang_Cursor_isNull(referenced_cursor) && !clang_isInvalid(clang_getCursorKind(referenced_cursor)))
        cursor = referenced_cursor;

    CXCursor canonical_cursor = clang_getCanonicalCursor(cursor);
    if (clang_Cursor_isNull(canonical_cursor) || clang_isInvalid(clang_getCursorKind(canonical_cursor)))
        return "{}";

    cxstring_ptr brief = clang_Cursor_getBriefCommentText(canonical_cursor);
    if (clang_getCString(brief))
        ss << clang_getCString(brief);

    // Write the footer.
    ss << "'}";
    vimson = ss.str();
    return vimson.c_str();
}

const char* libclang_vim::get_deduced_declaration_at(const location_tuple& location_info)
{
    static std::string vimson;

    // Write the header.
    std::stringstream ss;
    ss << "{";

    // Write the actual comment.
    cxindex_ptr index = clang_createIndex(/*excludeDeclsFromPCH=*/1, /*displayDiagnostics=*/0);

    std::string file_name = location_info.file;
    std::vector<const char*> args_ptrs = get_args_ptrs(location_info.args);
    cxtranslation_unit_ptr translation_unit(clang_parseTranslationUnit(index, file_name.c_str(), args_ptrs.data(), args_ptrs.size(), nullptr, 0, CXTranslationUnit_Incomplete));
    if (!translation_unit)
        return "{}";

    const CXFile file = clang_getFile(translation_unit, file_name.c_str());
    int line = location_info.line;
    int column = location_info.col;
    CXSourceLocation source_location = clang_getLocation(translation_unit, file, line, column);
    CXCursor cursor = clang_getCursor(translation_unit, source_location);
    if (clang_Cursor_isNull(cursor) || clang_isInvalid(clang_getCursorKind(cursor)))
        return "{}";

    CXCursor referenced_cursor = clang_getCursorReferenced(cursor);
    if (clang_Cursor_isNull(referenced_cursor) && clang_isInvalid(clang_getCursorKind(referenced_cursor)))
        return "{}";

    CXCursor canonical_cursor = clang_getCanonicalCursor(referenced_cursor);
    if (clang_Cursor_isNull(canonical_cursor) || clang_isInvalid(clang_getCursorKind(canonical_cursor)))
        return "{}";

    CXSourceLocation declaration_location = clang_getCursorLocation(canonical_cursor);
    CXFile declaration_file;
    unsigned declaration_line;
    unsigned declaration_col;
    clang_getExpansionLocation(declaration_location, &declaration_file, &declaration_line, &declaration_col, nullptr);
    cxstring_ptr declaration_file_name = clang_getFileName(declaration_file);
    ss << "'file':'" << clang_getCString(declaration_file_name)<<"',";
    ss << "'line':'" << declaration_line <<"',";
    ss << "'col':'" << declaration_col <<"',";

    // Write the footer.
    ss << "}";
    vimson = ss.str();
    return vimson.c_str();
}

const char* libclang_vim::get_include_at(const location_tuple& location_info)
{
    static std::string vimson;

    // Write the header.
    std::stringstream ss;
    ss << "{'file':'";

    // Write the actual comment.
    cxindex_ptr index = clang_createIndex(/*excludeDeclsFromPCH=*/1, /*displayDiagnostics=*/0);

    std::string file_name = location_info.file;
    std::vector<const char*> args_ptrs = get_args_ptrs(location_info.args);
    unsigned options = CXTranslationUnit_Incomplete | CXTranslationUnit_DetailedPreprocessingRecord;
    cxtranslation_unit_ptr translation_unit(clang_parseTranslationUnit(index, file_name.c_str(), args_ptrs.data(), args_ptrs.size(), nullptr, 0, options));
    if (!translation_unit)
        return "{}";

    const CXFile file = clang_getFile(translation_unit, file_name.c_str());
    int line = location_info.line;
    int column = location_info.col;
    CXSourceLocation source_location = clang_getLocation(translation_unit, file, line, column);
    CXCursor cursor = clang_getCursor(translation_unit, source_location);
    if (clang_getCursorKind(cursor) != CXCursor_InclusionDirective)
        return "{}";

    CXFile included_file = clang_getIncludedFile(cursor);
    cxstring_ptr included_name = clang_getFileName(included_file);
    ss << clang_getCString(included_name);

    // Write the footer.
    ss << "'}";
    vimson = ss.str();
    return vimson.c_str();
}

const char* libclang_vim::get_completion_at(const location_tuple& location_info)
{
    static std::string vimson;

    // Write the header.
    std::stringstream ss;
    ss << "['";

    // Write the completion list.
    libclang_vim::cxindex_ptr index = clang_createIndex(/*excludeDeclsFromPCH=*/1, /*displayDiagnostics=*/0);

    std::string file_name = location_info.file;
    std::vector<const char*> args_ptrs = get_args_ptrs(location_info.args);
    cxtranslation_unit_ptr translation_unit(clang_parseTranslationUnit(index, file_name.c_str(), args_ptrs.data(), args_ptrs.size(), nullptr, 0, CXTranslationUnit_Incomplete));
    if (!translation_unit)
        return "[]";

    unsigned line = location_info.line;
    unsigned column = location_info.col;
    CXCodeCompleteResults* results = clang_codeCompleteAt(translation_unit, file_name.c_str(), line, column, nullptr, 0, clang_defaultCodeCompleteOptions());
    std::set<std::string> matches;
    if (results)
    {
        for (unsigned i = 0; i < results->NumResults; ++i)
        {
            const CXCompletionString& completion_string = results->Results[i].CompletionString;
            std::stringstream match;
            for (unsigned j = 0; j < clang_getNumCompletionChunks(completion_string); ++j)
            {
                if (clang_getCompletionChunkKind(completion_string, j) != CXCompletionChunk_TypedText)
                    continue;

                const CXString& chunk_text = clang_getCompletionChunkText(completion_string, j);
                match << clang_getCString(chunk_text);
            }
            matches.insert(match.str());
        }
        clang_disposeCodeCompleteResults(results);
    }
    for (auto it = matches.begin(); it != matches.end(); ++it)
    {
        if (it != matches.begin())
            ss << "', '";
        ss << *it;
    }

    // Write the footer.
    ss << "']";
    vimson = ss.str();
    return vimson.c_str();
}

const char* libclang_vim::get_diagnostics(const std::pair<std::string, args_type>& file_and_args)
{
    static std::string vimson;

    // Write the header.
    std::stringstream ss;
    ss << "[";

    // Write the diagnostic list.
    libclang_vim::cxindex_ptr index = clang_createIndex(/*excludeDeclsFromPCH=*/1, /*displayDiagnostics=*/0);

    std::string file_name = file_and_args.first;
    std::vector<const char*> args_ptrs = get_args_ptrs(file_and_args.second);
    cxtranslation_unit_ptr translation_unit(clang_parseTranslationUnit(index, file_name.c_str(), args_ptrs.data(), args_ptrs.size(), nullptr, 0, CXTranslationUnit_Incomplete));
    if (!translation_unit)
        return "[]";

    unsigned num_diagnostics = clang_getNumDiagnostics(translation_unit);
    for (unsigned i = 0; i < num_diagnostics; ++i)
    {
        CXDiagnostic diagnostic = clang_getDiagnostic(translation_unit, i);
        if (diagnostic)
        {
            std::string severity;
            switch (clang_getDiagnosticSeverity(diagnostic))
            {
            case CXDiagnostic_Ignored:
                severity = "ignored";
                break;
            case CXDiagnostic_Note:
                severity = "note";
                break;
            case CXDiagnostic_Warning:
                severity = "warning";
                break;
            case CXDiagnostic_Error:
                severity = "error";
                break;
            case CXDiagnostic_Fatal:
                severity = "fatal";
                break;
            }
            ss << "{'severity': '" << severity << "', ";

            CXSourceLocation location = clang_getDiagnosticLocation(diagnostic);
            CXFile location_file;
            unsigned location_line;
            unsigned location_column;
            clang_getExpansionLocation(location, &location_file, &location_line, &location_column, nullptr);
            libclang_vim::cxstring_ptr location_file_name = clang_getFileName(location_file);
            ss << libclang_vim::stringize_location(location) << "}, ";
        }
        clang_disposeDiagnostic(diagnostic);
    }

    // Write the footer.
    ss << "]";
    vimson = ss.str();
    return vimson.c_str();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

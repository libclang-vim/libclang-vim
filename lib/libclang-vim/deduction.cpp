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

char const* libclang_vim::get_compile_commands(const std::string& file)
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

char const* libclang_vim::get_completion_at(location_tuple const& location_info)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

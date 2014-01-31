function! libclang#deduction#type_of_variable_declaration_at(filename, line, col)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, 'vim_clang_deduce_var_decl_at', printf("%s:%d:%d", a:filename, a:line, a:col)))
endfunction

function! libclang#deduction#type_of_function_declaration_at(filename, line, col)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, 'vim_clang_deduce_func_decl_at', printf("%s:%d:%d", a:filename, a:line, a:col)))
endfunction

function! libclang#deduction#type_of_function_or_variable_declaration_at(filename, line, col)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, 'vim_clang_deduce_func_or_var_decl_at', printf("%s:%d:%d", a:filename, a:line, a:col)))
endfunction



function! libclang#deduction#type_of_variable_declaration(filename, line, col)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, 'vim_clang_deduct_var_decl_at', printf("%s:%d:%d", a:filename, a:line, a:col)))
endfunction


function! libclang#location#information(filename, line, col)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, 'vim_clang_get_location_information', printf("%s:%d:%d", a:filename, a:line, a:col)))
endfunction

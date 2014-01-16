function! libclang#location#information(filename, line, col)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, 'vim_clang_get_location_information', printf("%s:%d:%d", a:filename, a:line, a:col)))
endfunction

function! libclang#location#extent(filename, line, col)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, 'vim_clang_get_extent_of_specific_location', printf("%s:%d:%d", a:filename, a:line, a:col)))
endfunction

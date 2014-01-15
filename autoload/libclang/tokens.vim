function! libclang#tokens#all(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, 'vim_clang_tokens', a:filename))
endfunction


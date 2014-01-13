let s:lib_path = g:libclang#lib_path

function! libclang#tokens#all(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_tokens', a:filename))
endfunction


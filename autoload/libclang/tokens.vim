function! libclang#tokens#all(filename)
    return libclang#call('vim_clang_tokens', a:filename)
endfunction

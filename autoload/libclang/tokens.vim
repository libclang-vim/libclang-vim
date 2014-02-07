function! libclang#tokens#all(file_name, ...)
    return libclang#call('vim_clang_tokens', a:file_name, a:000)
endfunction

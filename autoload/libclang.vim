let libclang#lib_path = expand('<sfile>:p:h:h') . '/lib/libclang-vim.so'

if ! filereadable(libclang#lib_path)
    echoerr 'libclang-vim: ' . libclang#lib_path . ' is not found! Please execute `make` in ' . expand('<sfile>:p:h:h')
    finish
endif

function! libclang#version()
    return libcall(libclang#lib_path, 'vim_clang_version', '')
endfunction

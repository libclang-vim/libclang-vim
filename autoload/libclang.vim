let g:libclang#lib_path = expand('<sfile>:p:h:h') . '/lib/libclang-vim.so'

if ! filereadable(g:libclang#lib_path)
    echoerr 'libclang-vim: ' . g:libclang#lib_path . ' is not found! Please execute `make` in ' . expand('<sfile>:p:h:h')
endif

function! libclang#version()
    return libcall(g:libclang#lib_path, 'vim_clang_version', '')
endfunction

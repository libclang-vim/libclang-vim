let s:libclang = expand('<sfile>:p:h:h') . '/lib/libclang-vim.so'

if ! filereadable(s:libclang)
    echoerr 'libclang-vim: ' . s:libclang . ' is not found! Please execute `make` in ' . expand('<sfile>:p:h:h')
    finish
endif

function! clang#version()
    return libcall(s:libclang, 'vim_clang_version', '')
endfunction

function! clang#path_to_libclang_vim()
    return s:libclang
endfunction

function! clang#tokens(filename)
    return eval(libcall(s:libclang, 'vim_clang_tokens', a:filename))
endfunction

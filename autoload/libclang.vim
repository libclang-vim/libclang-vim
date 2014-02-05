let g:libclang#lib_path = expand('<sfile>:p:h:h') . '/lib/libclang-vim.so'

if ! filereadable(g:libclang#lib_path)
    echoerr 'libclang-vim: ' . g:libclang#lib_path . ' is not found! Please execute `make` in ' . expand('<sfile>:p:h:h')
endif

function! libclang#version()
    return libcall(g:libclang#lib_path, 'vim_clang_version', '')
endfunction

function! libclang#call(api, file)
    if ! filereadable(a:file)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, a:api, a:file))
endfunction

function! libclang#call_at(api, file, line, col)
    if ! filereadable(a:file)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, a:api, printf("%s:%d:%d", a:file, a:line, a:col)))
endfunction

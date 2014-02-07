let g:libclang#lib_path = expand('<sfile>:p:h:h') . '/lib/libclang-vim.so'

let s:LIST_TYPE = type([])
let s:STRING_TYPE = type('')

if ! filereadable(g:libclang#lib_path)
    echoerr 'libclang-vim: ' . g:libclang#lib_path . ' is not found! Please execute `make` in ' . expand('<sfile>:p:h:h')
endif

function! libclang#version()
    return libcall(g:libclang#lib_path, 'vim_clang_version', '')
endfunction

function! s:get_extra_string(extra)
    if len(a:extra) == 1
        if type(a:extra[0]) == s:LIST_TYPE
            return join(a:extra[0], ' ')
        elseif type(a:extra[0]) == s:STRING_TYPE
            return a:extra[0]
        else
            return string(a:extra[0])
        endif
    else
        return ""
    endif
endfunction

function! libclang#call(api, file, extra)
    if ! filereadable(a:file)
        return {}
    endif
    let compiler_args = s:get_extra_string(a:extra)
    return eval(libcall(g:libclang#lib_path, a:api, a:file . ':' . compiler_args))
endfunction

function! libclang#call_at(api, file, line, col, extra)
    if ! filereadable(a:file)
        return {}
    endif
    let compiler_args = s:get_extra_string(a:extra)
    return eval(libcall(g:libclang#lib_path, a:api, printf("%s:%s:%d:%d", a:file, compiler_args, a:line, a:col)))
endfunction

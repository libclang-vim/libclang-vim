let g:libclang#lib_path = expand('<sfile>:p:h:h') . '/lib/libclang-vim.so'

let s:LIST_TYPE = type([])
let s:STRING_TYPE = type('')

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

function! libclang#call_at(api, file, line, col, extra)
    if ! filereadable(a:file)
        return {}
    endif

    if ! empty(a:extra)
        if type(a:extra) == s:LIST_TYPE
            let compiler_args = join(a:extra[0], ' ')
        elseif type(a:extra) == s:STRING_TYPE
            let compiler_args = a:extra[0]
        else
            let compiler_args = string(a:extra[0])
        endif
    else
        let compiler_args = ""
    endif

    return eval(libcall(g:libclang#lib_path, a:api, printf("%s:%s:%d:%d", a:file, compiler_args, a:line, a:col)))
endfunction

let s:libclang = expand('<sfile>:p:h:h') . '/lib/libclang-vim.so'

if ! filereadable(s:libclang)
    echoerr 'libclang-vim: ' . s:libclang . ' is not found! Please execute `make` in ' . expand('<sfile>:p:h:h')
    finish
endif

function! libclang#version()
    return libcall(s:libclang, 'vim_clang_version', '')
endfunction

function! libclang#path_to_libclang_vim()
    return s:libclang
endfunction

function! libclang#tokens(filename)
    if ! filereadable(a:filename)
        echoerr "libclang: File not found: " . a:filename
        return {}
    endif
    return eval(libcall(s:libclang, 'vim_clang_tokens', a:filename))
endfunction

function! libclang#AST(filename)
    if ! filereadable(a:filename)
        echoerr "libclang: File not found: " . a:filename
        return {}
    endif
    return eval(libcall(s:libclang, 'vim_clang_build_AST', a:filename))
endfunction

function! libclang#declarations(filename)
    if ! filereadable(a:filename)
        echoerr "libclang: File not found: " . a:filename
        return {}
    endif
    return eval(libcall(s:libclang, 'vim_clang_extract_declarations', a:filename))
endfunction

function! libclang#attributes(filename)
    if ! filereadable(a:filename)
        echoerr "libclang: File not found: " . a:filename
        return {}
    endif
    return eval(libcall(s:libclang, 'vim_clang_extract_attributes', a:filename))
endfunction

function! libclang#expressions(filename)
    if ! filereadable(a:filename)
        echoerr "libclang: File not found: " . a:filename
        return {}
    endif
    return eval(libcall(s:libclang, 'vim_clang_extract_expressions', a:filename))
endfunction

function! libclang#preprocessings(filename)
    if ! filereadable(a:filename)
        echoerr "libclang: File not found: " . a:filename
        return {}
    endif
    return eval(libcall(s:libclang, 'vim_clang_extract_preprocessings', a:filename))
endfunction

function! libclang#references(filename)
    if ! filereadable(a:filename)
        echoerr "libclang: File not found: " . a:filename
        return {}
    endif
    return eval(libcall(s:libclang, 'vim_clang_extract_references', a:filename))
endfunction

function! libclang#statements(filename)
    if ! filereadable(a:filename)
        echoerr "libclang: File not found: " . a:filename
        return {}
    endif
    return eval(libcall(s:libclang, 'vim_clang_extract_statements', a:filename))
endfunction

function! libclang#translation_units(filename)
    if ! filereadable(a:filename)
        echoerr "libclang: File not found: " . a:filename
        return {}
    endif
    return eval(libcall(s:libclang, 'vim_clang_extract_translation_units', a:filename))
endfunction

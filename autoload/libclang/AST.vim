let s:lib_path = libclang#lib_path

function! libclang#AST#all(filename)
    if ! filereadable(a:filename)
        echoerr "libclang: File not found: " . a:filename
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_build_AST', a:filename))
endfunction

function! libclang#AST#declarations(filename)
    if ! filereadable(a:filename)
        echoerr "libclang: File not found: " . a:filename
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_declarations', a:filename))
endfunction

function! libclang#AST#attributes(filename)
    if ! filereadable(a:filename)
        echoerr "libclang: File not found: " . a:filename
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_attributes', a:filename))
endfunction

function! libclang#AST#expressions(filename)
    if ! filereadable(a:filename)
        echoerr "libclang: File not found: " . a:filename
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_expressions', a:filename))
endfunction

function! libclang#AST#preprocessings(filename)
    if ! filereadable(a:filename)
        echoerr "libclang: File not found: " . a:filename
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_preprocessings', a:filename))
endfunction

function! libclang#AST#references(filename)
    if ! filereadable(a:filename)
        echoerr "libclang: File not found: " . a:filename
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_references', a:filename))
endfunction

function! libclang#AST#statements(filename)
    if ! filereadable(a:filename)
        echoerr "libclang: File not found: " . a:filename
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_statements', a:filename))
endfunction

function! libclang#AST#translation_units(filename)
    if ! filereadable(a:filename)
        echoerr "libclang: File not found: " . a:filename
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_translation_units', a:filename))
endfunction

let s:lib_path = g:libclang#lib_path

function! libclang#AST#all(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_all', a:filename))
endfunction

" TODO: DRY using meta programming

function! libclang#AST#declarations(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_declarations', a:filename))
endfunction

function! libclang#AST#attributes(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_attributes', a:filename))
endfunction

function! libclang#AST#expressions(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_expressions', a:filename))
endfunction

function! libclang#AST#preprocessings(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_preprocessings', a:filename))
endfunction

function! libclang#AST#references(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_references', a:filename))
endfunction

function! libclang#AST#statements(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_statements', a:filename))
endfunction

function! libclang#AST#translation_units(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_translation_units', a:filename))
endfunction

function! libclang#AST#definitions(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_definitions', a:filename))
endfunction

function! libclang#AST#virtual_member_functions(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_virtual_member_functions', a:filename))
endfunction

function! libclang#AST#pure_virtual_member_functions(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_pure_virtual_member_functions', a:filename))
endfunction

function! libclang#AST#static_member_functions(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_static_member_functions', a:filename))
endfunction

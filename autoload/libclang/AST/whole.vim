let s:lib_path = g:libclang#lib_path

function! libclang#AST#whole#all(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_all', a:filename))
endfunction

" TODO: DRY using meta programming

function! libclang#AST#whole#declarations(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_declarations', a:filename))
endfunction

function! libclang#AST#whole#attributes(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_attributes', a:filename))
endfunction

function! libclang#AST#whole#expressions(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_expressions', a:filename))
endfunction

function! libclang#AST#whole#preprocessings(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_preprocessings', a:filename))
endfunction

function! libclang#AST#whole#references(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_references', a:filename))
endfunction

function! libclang#AST#whole#statements(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_statements', a:filename))
endfunction

function! libclang#AST#whole#translation_units(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_translation_units', a:filename))
endfunction

function! libclang#AST#whole#definitions(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_definitions', a:filename))
endfunction

function! libclang#AST#whole#virtual_member_functions(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_virtual_member_functions', a:filename))
endfunction

function! libclang#AST#whole#pure_virtual_member_functions(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_pure_virtual_member_functions', a:filename))
endfunction

function! libclang#AST#whole#static_member_functions(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_static_member_functions', a:filename))
endfunction

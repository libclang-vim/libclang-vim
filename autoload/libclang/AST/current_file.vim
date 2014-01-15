let s:lib_path = g:libclang#lib_path

function! libclang#AST#current_file#all(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_all_current_file', a:filename))
endfunction

function! libclang#AST#current_file#declarations(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_declarations_current_file', a:filename))
endfunction

function! libclang#AST#current_file#attributes(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_attributes_current_file', a:filename))
endfunction

function! libclang#AST#current_file#expressions(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_expressions_current_file', a:filename))
endfunction

function! libclang#AST#current_file#preprocessings(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_preprocessings_current_file', a:filename))
endfunction

function! libclang#AST#current_file#references(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_references_current_file', a:filename))
endfunction

function! libclang#AST#current_file#statements(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_statements_current_file', a:filename))
endfunction

function! libclang#AST#current_file#translation_units(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_translation_units_current_file', a:filename))
endfunction

function! libclang#AST#current_file#definitions(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_definitions_current_file', a:filename))
endfunction

function! libclang#AST#current_file#virtual_member_functions(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_virtual_member_functions_current_file', a:filename))
endfunction

function! libclang#AST#current_file#pure_virtual_member_functions(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_pure_virtual_member_functions_current_file', a:filename))
endfunction

function! libclang#AST#current_file#static_member_functions(filename)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(s:lib_path, 'vim_clang_extract_static_member_functions_current_file', a:filename))
endfunction

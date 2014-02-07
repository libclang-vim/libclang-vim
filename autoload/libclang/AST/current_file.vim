function! libclang#AST#current_file#all(filename, ...)
    return libclang#call('vim_clang_extract_all_current_file', a:filename, a:000)
endfunction
function! libclang#AST#current_file#declarations(filename, ...)
    return libclang#call('vim_clang_extract_declarations_current_file', a:filename, a:000)
endfunction
function! libclang#AST#current_file#attributes(filename, ...)
    return libclang#call('vim_clang_extract_attributes_current_file', a:filename, a:000)
endfunction
function! libclang#AST#current_file#expressions(filename, ...)
    return libclang#call('vim_clang_extract_expressions_current_file', a:filename, a:000)
endfunction
function! libclang#AST#current_file#preprocessings(filename, ...)
    return libclang#call('vim_clang_extract_preprocessings_current_file', a:filename, a:000)
endfunction
function! libclang#AST#current_file#references(filename, ...)
    return libclang#call('vim_clang_extract_references_current_file', a:filename, a:000)
endfunction
function! libclang#AST#current_file#statements(filename, ...)
    return libclang#call('vim_clang_extract_statements_current_file', a:filename, a:000)
endfunction
function! libclang#AST#current_file#translation_units(filename, ...)
    return libclang#call('vim_clang_extract_translation_units_current_file', a:filename, a:000)
endfunction
function! libclang#AST#current_file#definitions(filename, ...)
    return libclang#call('vim_clang_extract_definitions_current_file', a:filename, a:000)
endfunction
function! libclang#AST#current_file#virtual_member_functions(filename, ...)
    return libclang#call('vim_clang_extract_virtual_member_functions_current_file', a:filename, a:000)
endfunction
function! libclang#AST#current_file#pure_virtual_member_functions(filename, ...)
    return libclang#call('vim_clang_extract_pure_virtual_member_functions_current_file', a:filename, a:000)
endfunction
function! libclang#AST#current_file#static_member_functions(filename, ...)
    return libclang#call('vim_clang_extract_static_member_functions_current_file', a:filename, a:000)
endfunction

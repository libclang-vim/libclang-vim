function! libclang#AST#non_system_headers#all(filename, ...)
    return libclang#call('vim_clang_extract_all_non_system_headers', a:filename, a:000)
endfunction
function! libclang#AST#non_system_headers#declarations(filename, ...)
    return libclang#call('vim_clang_extract_declarations_non_system_headers', a:filename, a:000)
endfunction
function! libclang#AST#non_system_headers#attributes(filename, ...)
    return libclang#call('vim_clang_extract_attributes_non_system_headers', a:filename, a:000)
endfunction
function! libclang#AST#non_system_headers#expressions(filename, ...)
    return libclang#call('vim_clang_extract_expressions_non_system_headers', a:filename, a:000)
endfunction
function! libclang#AST#non_system_headers#preprocessings(filename, ...)
    return libclang#call('vim_clang_extract_preprocessings_non_system_headers', a:filename, a:000)
endfunction
function! libclang#AST#non_system_headers#references(filename, ...)
    return libclang#call('vim_clang_extract_references_non_system_headers', a:filename, a:000)
endfunction
function! libclang#AST#non_system_headers#statements(filename, ...)
    return libclang#call('vim_clang_extract_statements_non_system_headers', a:filename, a:000)
endfunction
function! libclang#AST#non_system_headers#translation_units(filename, ...)
    return libclang#call('vim_clang_extract_translation_units_non_system_headers', a:filename, a:000)
endfunction
function! libclang#AST#non_system_headers#definitions(filename, ...)
    return libclang#call('vim_clang_extract_definitions_non_system_headers', a:filename, a:000)
endfunction
function! libclang#AST#non_system_headers#virtual_member_functions(filename, ...)
    return libclang#call('vim_clang_extract_virtual_member_functions_non_system_headers', a:filename, a:000)
endfunction
function! libclang#AST#non_system_headers#pure_virtual_member_functions(filename, ...)
    return libclang#call('vim_clang_extract_pure_virtual_member_functions_non_system_headers', a:filename, a:000)
endfunction
function! libclang#AST#non_system_headers#static_member_functions(filename, ...)
    return libclang#call('vim_clang_extract_static_member_functions_non_system_headers', a:filename, a:000)
endfunction

function! libclang#AST#whole#all(filename, ...)
    return libclang#call('vim_clang_extract_all', a:filename, a:000)
endfunction
function! libclang#AST#whole#declarations(filename, ...)
    return libclang#call('vim_clang_extract_declarations', a:filename, a:000)
endfunction
function! libclang#AST#whole#attributes(filename, ...)
    return libclang#call('vim_clang_extract_attributes', a:filename, a:000)
endfunction
function! libclang#AST#whole#expressions(filename, ...)
    return libclang#call('vim_clang_extract_expressions', a:filename, a:000)
endfunction
function! libclang#AST#whole#preprocessings(filename, ...)
    return libclang#call('vim_clang_extract_preprocessings', a:filename, a:000)
endfunction
function! libclang#AST#whole#references(filename, ...)
    return libclang#call('vim_clang_extract_references', a:filename, a:000)
endfunction
function! libclang#AST#whole#statements(filename, ...)
    return libclang#call('vim_clang_extract_statements', a:filename, a:000)
endfunction
function! libclang#AST#whole#translation_units(filename, ...)
    return libclang#call('vim_clang_extract_translation_units', a:filename, a:000)
endfunction
function! libclang#AST#whole#definitions(filename, ...)
    return libclang#call('vim_clang_extract_definitions', a:filename, a:000)
endfunction
function! libclang#AST#whole#virtual_member_functions(filename, ...)
    return libclang#call('vim_clang_extract_virtual_member_functions', a:filename, a:000)
endfunction
function! libclang#AST#whole#pure_virtual_member_functions(filename, ...)
    return libclang#call('vim_clang_extract_pure_virtual_member_functions', a:filename, a:000)
endfunction
function! libclang#AST#whole#static_member_functions(filename, ...)
    return libclang#call('vim_clang_extract_static_member_functions', a:filename, a:000)
endfunction

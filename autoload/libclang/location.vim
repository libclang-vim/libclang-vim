function! libclang#location#AST_node(filename, line, col)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, 'vim_clang_get_location_information', printf("%s:%d:%d", a:filename, a:line, a:col)))
endfunction

function! libclang#location#extent(filename, line, col)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, 'vim_clang_get_extent_of_node_at_specific_location', printf("%s:%d:%d", a:filename, a:line, a:col)))
endfunction

function! libclang#location#expression_extent(filename, line, col)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, 'vim_clang_get_expression_extent_at_specific_location', printf("%s:%d:%d", a:filename, a:line, a:col)))
endfunction

function! libclang#location#statement_extent(filename, line, col)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, 'vim_clang_get_statement_extent_at_specific_location', printf("%s:%d:%d", a:filename, a:line, a:col)))
endfunction

function! libclang#location#function_extent(filename, line, col)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, 'vim_clang_get_function_extent_at_specific_location', printf("%s:%d:%d", a:filename, a:line, a:col)))
endfunction

function! libclang#location#class_extent(filename, line, col)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, 'vim_clang_get_class_extent_at_specific_location', printf("%s:%d:%d", a:filename, a:line, a:col)))
endfunction

function! libclang#location#parameter_extent(filename, line, col)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, 'vim_clang_get_parameter_extent_at_specific_location', printf("%s:%d:%d", a:filename, a:line, a:col)))
endfunction

function! libclang#location#namespace_extent(filename, line, col)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, 'vim_clang_get_namespace_extent_at_specific_location', printf("%s:%d:%d", a:filename, a:line, a:col)))
endfunction

function! libclang#location#inner_definition_extent(filename, line, col)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, 'vim_clang_get_inner_definition_extent_at_specific_location', printf("%s:%d:%d", a:filename, a:line, a:col)))
endfunction

function! libclang#location#definition_at(filename, line, col)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, 'vim_clang_get_definition_at', printf("%s:%d:%d", a:filename, a:line, a:col)))
endfunction

function! libclang#location#referenced_at(filename, line, col)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, 'vim_clang_get_referenced_at', printf("%s:%d:%d", a:filename, a:line, a:col)))
endfunction

function! libclang#location#declaration_at(filename, line, col)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, 'vim_clang_get_declaration_at', printf("%s:%d:%d", a:filename, a:line, a:col)))
endfunction

function! libclang#location#pointee_type_at(filename, line, col)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, 'vim_clang_get_pointee_type_at', printf("%s:%d:%d", a:filename, a:line, a:col)))
endfunction

function! libclang#location#canonical_type_at(filename, line, col)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, 'vim_clang_get_canonical_type_at', printf("%s:%d:%d", a:filename, a:line, a:col)))
endfunction

function! libclang#location#result_type_at(filename, line, col)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, 'vim_clang_get_result_type_at', printf("%s:%d:%d", a:filename, a:line, a:col)))
endfunction

function! libclang#location#class_type_of_member_pointer_at(filename, line, col)
    if ! filereadable(a:filename)
        return {}
    endif
    return eval(libcall(g:libclang#lib_path, 'vim_clang_get_class_type_of_member_pointer_at', printf("%s:%d:%d", a:filename, a:line, a:col)))
endfunction

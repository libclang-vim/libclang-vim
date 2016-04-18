function! libclang#deduction#type_of_variable_declaration_at(filename, line, col, ...)
    return libclang#call_at('vim_clang_deduce_var_decl_at', a:filename, a:line, a:col, a:000)
endfunction
function! libclang#deduction#type_of_function_declaration_at(filename, line, col, ...)
    return libclang#call_at('vim_clang_deduce_func_decl_at', a:filename, a:line, a:col, a:000)
endfunction
function! libclang#deduction#type_of_function_or_variable_declaration_at(filename, line, col, ...)
    return libclang#call_at('vim_clang_deduce_func_or_var_decl_at', a:filename, a:line, a:col, a:000)
endfunction
function! libclang#deduction#type_at(filename, line, col, ...)
    return libclang#call_at('vim_clang_get_type_with_deduction_at', a:filename, a:line, a:col, a:000)
endfunction
function! libclang#deduction#current_function_at(filename, line, col, ...)
    return libclang#call_at('vim_clang_get_current_function_at', a:filename, a:line, a:col, a:000)
endfunction
function! libclang#deduction#completion_at(filename, line, col, ...)
    return libclang#call_at('vim_clang_get_completion_at', a:filename, a:line, a:col, a:000)
endfunction
function! libclang#deduction#comment_at(filename, line, col, ...)
    return libclang#call_at('vim_clang_get_comment_at', a:filename, a:line, a:col, a:000)
endfunction
function! libclang#deduction#declaration_at(filename, line, col, ...)
    return libclang#call_at('vim_clang_get_deduced_declaration_at', a:filename, a:line, a:col, a:000)
endfunction
function! libclang#deduction#compile_commands(filename)
    return libclang#call('vim_clang_get_compile_commands', a:filename, "")
endfunction


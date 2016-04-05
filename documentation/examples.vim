"
" Sample functions that demonstrates how the libclang-vim API can be used.
"

" The API works with files, but the user works with buffers. This function
" converts a buffer to a temp file.
function! ClangTempFile()
    let temp_name = tempname() . (&filetype==#'c' ? '.c' : '.cpp')
    if writefile(getbufline(bufnr('%'), 1, '$'), temp_name) == -1
        throw "Could not create a temporary file : ".temp_name
    endif
    return temp_name
endfunction

" Example for libclang#deduction#type_at().
function! ClangInspectType()
    let compiler_args = libclang#deduction#compile_commands(expand('%:p'))
    let file_name = ClangTempFile()
    let type_info = libclang#deduction#type_at(file_name, line('.'), col('.'), compiler_args.commands)
    call delete(file_name)
    " Use '.canonical.type' instead of '.type' if you want e.g.
    " 'std::basic_string<char>', not 'std::string'.
    echo type_info.type
endfunction

" Example for libclang#deduction#current_function_at().
function! ClangInspectFunction()
    let compiler_args = libclang#deduction#compile_commands(expand('%:p'))
    let file_name = ClangTempFile()
    let info = libclang#deduction#current_function_at(file_name, line('.'), col('.'), compiler_args.commands)
    call delete(file_name)
    echo info.name
endfunction

" Example for libclang#deduction#completion_at().
function! ClangInspectCompletion(findstart, base)
    if a:findstart == 1
        " In findstart mode, look for the beginning of the current identifier.
        let l:line = getline('.')
        let l:start = col('.') - 1
        while l:start > 0 && l:line[l:start - 1] =~ '\i'
            let l:start -= 1
        endwhile
        return l:start
    endif

    " Get the current line and column numbers.
    let l:l = line('.')
    let l:c = col('.')

    let compiler_args = libclang#deduction#compile_commands(expand('%:p'))

    let file_name = ClangTempFile()
    let l:matches = libclang#deduction#completion_at(file_name, line('.'), col('.'), compiler_args.commands)
    call delete(file_name)

    " Filter out matches that do not match the prefix we got.
    let l:ret = l:matches
    if a:base != ""
        let l:ret = []
        for l:match in l:matches
            if l:match[:strlen(a:base)-1] == a:base
                let l:ret = l:ret + [l:match]
            endif
        endfor
    endif

    return l:ret
endfunction

" vim:set shiftwidth=4 softtabstop=4 expandtab:

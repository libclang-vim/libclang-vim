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
    let l:compiler_args = libclang#deduction#compile_commands(expand('%:p'))
    let l:temp_file = ClangTempFile()
    let l:file_name = expand('%:p') . '#' . l:temp_file
    let type_info = libclang#deduction#type_at(l:file_name, line('.'), col('.'), l:compiler_args.commands)
    call delete(l:temp_file)
    if type_info.type ==# type_info.canonical.type
        echo type_info.type
    else
        " E.g. std::string => std::basic_string<char>
        echo type_info.type . ' => ' . type_info.canonical.type
    endif
endfunction

" Example for libclang#deduction#current_function_at().
function! ClangInspectFunction()
    let l:compiler_args = libclang#deduction#compile_commands(expand('%:p'))
    let l:temp_file = ClangTempFile()
    let l:file_name = expand('%:p') . '#' . l:temp_file
    let l:info = libclang#deduction#current_function_at(l:file_name, line('.'), col('.'), l:compiler_args.commands)
    call delete(l:temp_file)
    echo l:info.name . '()'
endfunction

" Example for libclang#deduction#full_name_at().
function! ClangInspectName()
    let l:compiler_args = libclang#deduction#compile_commands(expand('%:p'))
    let l:temp_file = ClangTempFile()
    let l:file_name = expand('%:p') . '#' . l:temp_file
    let l:info = libclang#deduction#full_name_at(l:file_name, line('.'), col('.'), l:compiler_args.commands)
    call delete(l:temp_file)
    echo l:info.name
endfunction

" Example for libclang#deduction#comment_at().
function! ClangInspectComment()
    let l:compiler_args = libclang#deduction#compile_commands(expand('%:p'))
    let l:temp_file = ClangTempFile()
    let l:file_name = expand('%:p') . '#' . l:temp_file
    let l:info = libclang#deduction#comment_at(file_name, line('.'), col('.'), l:compiler_args.commands)
    call delete(l:temp_file)
    echo l:info.brief
endfunction

" Example for libclang#deduction#declaration_at().
" See ':help jumplist', e.g. use Ctrl-O to jump back.
function! ClangJumpDeclaration()
    let compiler_args = libclang#deduction#compile_commands(expand('%:p'))
    let temp_file = ClangTempFile()
    let file_name = expand('%:p') . '#' . temp_file
    let info = libclang#deduction#declaration_at(file_name, line('.'), col('.'), compiler_args.commands)
    call delete(temp_file)

    if info.file == file_name
        let info.file = expand('%:p')
    endif

    " Add an entry to the jump list.
    normal! m'
    " Avoid adding the file open to the jump list.
    execute "keepjumps edit " . info.file
    call cursor(info.line, info.col)
    " Move the cursor to the center of the screen.
    normal! zz
endfunction

" Example for libclang#deduction#include_at().
" See ':help jumplist', e.g. use Ctrl-O to jump back.
function! ClangJumpInclude()
    let compiler_args = libclang#deduction#compile_commands(expand('%:p'))
    let temp_file = ClangTempFile()
    let file_name = expand('%:p') . '#' . temp_file
    let info = libclang#deduction#include_at(file_name, line('.'), col('.'), compiler_args.commands)
    call delete(temp_file)

    " Add an entry to the jump list.
    normal! m'
    execute "edit " . info.file
endfunction

" Example for libclang#deduction#diagnostics().
sign define ClangError text=EE
sign define ClangWarning text=WW
function! ClangShowDiagnostics()
    let l:compiler_args = libclang#deduction#compile_commands(expand('%:p'))
    let l:temp_file = ClangTempFile()
    let l:file_name = expand('%:p') . '#' . l:temp_file
    let l:diagnostics = libclang#deduction#diagnostics(l:file_name, l:compiler_args.commands)
    call delete(l:temp_file)

    " Delete previous diagnostics.
    sign unplace *
    let l:counter = 1
    for l:diagnostic in l:diagnostics
        if has_key(l:diagnostic, "file")
            if l:diagnostic.file == expand('%:p')
                " This diagnostic is for the current file.
                if l:diagnostic.severity == "warning"
                    execute "sign place " . l:counter . " line=" . l:diagnostic.line . " name=ClangWarning file=" . expand("%:p")
                    let l:counter += 1
                elseif l:diagnostic.severity == "error"
                    execute "sign place " . l:counter . " line=" . l:diagnostic.line . " name=ClangError file=" . expand("%:p")
                    let l:counter += 1
                endif
            endif
        endif
    endfor
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

    let l:temp_file = ClangTempFile()
    let l:file_name = expand('%:p') . '#' . l:temp_file
    let l:matches = libclang#deduction#completion_at(l:file_name, line('.'), col('.'), compiler_args.commands)
    call delete(l:temp_file)

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

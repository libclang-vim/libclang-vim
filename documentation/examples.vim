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
    if type_info.type ==# type_info.canonical.type
        echo type_info.type
    else
        " E.g. std::string => std::basic_string<char>
        echo type_info.type . ' => ' . type_info.canonical.type
    endif
endfunction

" Example for libclang#deduction#current_function_at().
function! ClangInspectFunction()
    let compiler_args = libclang#deduction#compile_commands(expand('%:p'))
    let file_name = ClangTempFile()
    let info = libclang#deduction#current_function_at(file_name, line('.'), col('.'), compiler_args.commands)
    call delete(file_name)
    echo info.name
endfunction

" Example for libclang#deduction#comment_at().
function! ClangInspectComment()
    let compiler_args = libclang#deduction#compile_commands(expand('%:p'))
    let file_name = ClangTempFile()
    let info = libclang#deduction#comment_at(file_name, line('.'), col('.'), compiler_args.commands)
    call delete(file_name)
    echo info.brief
endfunction

" Example for libclang#deduction#declaration_at().
" See ':help jumplist', e.g. use Ctrl-O to jump back.
function! ClangJumpDeclaration()
    let compiler_args = libclang#deduction#compile_commands(expand('%:p'))
    let file_name = expand('%:p') . '#' . ClangTempFile()
    let info = libclang#deduction#declaration_at(file_name, line('.'), col('.'), compiler_args.commands)
    call delete(file_name)

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
    let file_name = expand('%:p') . '#' . ClangTempFile()
    let info = libclang#deduction#include_at(file_name, line('.'), col('.'), compiler_args.commands)
    call delete(file_name)

    " Add an entry to the jump list.
    normal! m'
    execute "edit " . info.file
endfunction

" Example for libclang#deduction#diagnostics().
sign define ClangError text=EE
sign define ClangWarning text=WW
function! ClangShowDiagnostics()
    let l:compiler_args = libclang#deduction#compile_commands(expand('%:p'))
    let l:file_name = ClangTempFile()
    let l:diagnostics = libclang#deduction#diagnostics(l:file_name, l:compiler_args.commands)
    call delete(l:file_name)

    " Delete previous diagnostics.
    sign unplace *
    let l:counter = 1
    for l:diagnostic in l:diagnostics
        if has_key(l:diagnostic, "file")
            if l:diagnostic.file == l:file_name
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

" Example for libclang#deduction#compile_commands().
" Note that this naturally can be correct when changes are necessary only
" in the current buffer and nowhere else.
" Requires clang-tools-extra r267855 or newer.
function! ClangRename()
    " Save cursor position.
    let l:offset_orig = line2byte(line('.')) + col('.') - 1

    " Look for the beginning of the current identifier.
    let l:line = getline('.')
    let l:start = col('.') - 1
    while l:start > 0 && l:line[l:start - 1] =~ '\i'
        let l:start -= 1
    endwhile
    let l:offset = line2byte(line('.')) + l:start + 1

    " Ask for the new name.
    let l:to = input('Rename to: ')

    " Look up compiler arguments.
    let l:compiler_args = libclang#deduction#compile_commands(expand('%:p'))

    " Call clang-rename to get the old name and positions.
    let l:file_name = ClangTempFile()
    let l:command = 'clang-rename -pl -pn -offset ' . l:offset . ' -new-name ' . l:to . ' ' . l:file_name . ' -- ' . l:compiler_args.commands . ' 2>&1'
    let l:out = split(system(l:command), "\n")
    let l:positions = []
    for l:line in l:out
        if l:line[:13] == 'clang-rename: '
            let l:line = l:line[14:]
            if l:line[:11] == 'found name: '
                let l:old_name = l:line[12:]
            elseif l:line[:11] == 'renamed at: '
                let l:line = l:line[12:]
                let l:idx = stridx(l:line, ':')
                let l:row = l:line[ l:idx + 1 : ]
                let l:idx = stridx(l:row, ':')
                let l:position = {}
                let l:position.col = l:row[ l:idx + 1 : ]
                let l:position.row = l:row[ : l:idx - 1 ]
                let l:positions = l:positions + [l:position]
            endif
        endif
    endfor

    " Finally do the rename at the found places.
    for l:position in l:positions
        call cursor(l:position.row, l:position.col)
        execute "normal " . strlen(l:old_name) . "x"
        execute "normal i" . l:to . "\<Esc>"
    endfor

    call delete(file_name)

    execute "go " . l:offset_orig
endfunction

" vim:set shiftwidth=4 softtabstop=4 expandtab:

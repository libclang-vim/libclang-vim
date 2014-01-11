let s:libclang = expand('<sfile>:p:h:h') . '/lib/libclang-vim.so'

" let clang#llvm_config = get(g:, 'clang#llvm_config', executable('llvm-config-3.4') ? 'llvm-config-3.4' : executable('llvm-config') ? 'llvm-config' : '')
" if clang#llvm_config == ''
"     echoerror 'llvm-config command not found.'
"     finish
" endif
"
" let $LD_LIBRARY_PATH = substitute(system(clang#llvm_config . ' --libdir'), '\n\+$', '', '') . ':' . $LD_LIBRARY_PATH

if ! filereadable(s:libclang)
    echoerr 'libclang-vim: ' . s:libclang . ' is not found! Please execute `make` in ' . expand('<sfile>:p:h:h')
    finish
endif

function! clang#version()
    return libcall(s:libclang, 'vim_clang_version', '')
endfunction

function! clang#path_to_libclang_vim()
    return s:libclang
endfunction

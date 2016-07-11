Vim script Interface to Clang
===========================
[![Build Status](https://travis-ci.org/libclang-vim/libclang-vim.png?branch=master)](https://travis-ci.org/rhysd/libclang-vim)

libclang-vim is a wrapper library of [libclang](http://clang.llvm.org/doxygen/group__CINDEX.html) for Vim script developers who want to make a great C, C++ and Objective-C plugin.
You can use the power of clang's C, C++ and Objective-C code analysis from Vim script directly.
It enables to parse the code with an abstract syntax tree (AST).
It means considering the context of the code.
The analysis with AST is further powerful than the one with regular expressions.

You can

- get tokens of the code.
- extract AST information of the code.
- get information at the specific location of the code.
- get extent at the specific location of the code
- get definition, declaration and referenced node of the item at specific location
- get pointee type, result type, canonical type at specific location
- deduce type of variable declaration and return value of function
- get the information for completion.
- get diagnostic information.
- get preprocessing information.
- get comment information.


## Usage

In all below usages, `{compiler args}` means arguments passed to a compiler. (e.g. `"-std=c++1y"`)

Also, in all below usages, `{filename}` can be in the form of `{real
filename}#{temp filename}`, where the previous is compiler should take the path
of the first, and the contents of the later file (also known as unsaved file
support). This can be useful when the temp file is a dump of the editor buffer,
and passing the temp file directly to the compiler would not be possible due to
relative include paths.

### `libclang#version()`

Get version of libclang as a string.

### `libclang#tokens#all({filename} [, {compiler args}])`

Get tokens in `{filename}`.  It includes all tokens in included header files.

### `libclang#AST#{extent}#{kind of node}({filename} [, {compiler args}])`

Get information of a specific kind of node in AST as a dictionary.

`{extent}` is the extent of analysis. `whole` searches all the code, `non_system_headers` searches all the code except for system headers (which are included with `#include <>`), `current_file` searches the code in only the current file.

`{kind of node}` is a kind of AST nodes which you want to extract.  `all` extracts all kind of AST nodes, `declarations` extracts all AST nodes related to declarations, `definitions` extracts all AST nodes related to definitions, `expressions` extracts all AST node related to expressions, and so on.

If you want to get information about definitions and not to get AST information about system headers, you should use `libclang#AST#non_system_headers#definitions()`.

### `libclang#location#AST_node({filename}, {line}, {col} [, {compiler args}])`

Get the AST node information at specific location.

### `libclang#location#extent({filename}, {line}, {col} [, {compiler args}])`

Get the extent of the most inner syntax element at specific location.

### `libclang#location#{syntax element}_extent({filename}, {line}, {col} [, {compiler args}])`

Get the extent of `{syntax element}` at specific location.

`{syntax element}` is a kind of AST node and you can select one of below kinds.

- `expression`
- `statement`
- `function` : function, function template member function, member function template
- `class` : class, class template
- `parameter` : function parameter, template parameter, template template parameter
- `namespace`
- `inner_definition` : the most inner definition

If you want to get the extent of a class at specific location, you should use `libclang#location#class_extent()`.

You want to see actual input and output?  Please see below Example section.

### `libclang#location#{something}_at({filename}, {line}, {col} [, {compiler args}])`

Get the node information related at specific location.

`{something}` is one of below items.

- `definition` : definition node of node at specific location
- `referenced` : node which node at specific location references
- `declaration` : declaration node of node at specific location
- `pointee_type` : type of the pointer at specific location
- `canonical_type` : canonical type of the type at specific location
- `result_type` : result type at specific location (e.g. result type of function)
- `class_type_of_member_pointer` : class type of member function at specific location

If you want to get the definition of specific location, you should use `libclang#location#definition_at()`.
If you want to know what item specific location references, you should use `libclang#location#referenced_at()`.
If you want to get the type of function at specific location, you should use `libclang#locaiton#result_type_at()`.

### `libclang#deduction#type_of_function_or_variable_declaration({filename}, {line}, {col} [, {compiler args}])`

Deduce type of variable and return value of function at `{line}, {col}`.  You must specify `{line}` and `{col}` of variable declaration or function declaration.  If you specify the place of variable declaration and the type of variable is `auto`, it searches type of left hand side of the declaration.  And if you specify the place of function declaration whose return type is `auto`, it searches type of return statement in the function.

This manual detection will be obsolete when `clang_getCanonicalType()` API will be fixed.  Then I will simply use the API and remove manual detection. Reference links are below.

- http://clang-developers.42468.n3.nabble.com/API-for-auto-type-deduction-in-libclang-td4037350.html
- http://llvm.org/bugs/show_bug.cgi?id=18669

### `libclang#deduction#type_at({filename}, {line}, {col} [, {compiler args}])`

Get type at specific location with auto-deduction described above.

### `libclang#deduction#current_function_at({filename}, {line}, {col} [, {compiler args}])`

Get the name of the qualified name of the current function at specific location.

### `libclang#deduction#completion_at({filename}, {line}, {col} [, {compiler args}])`

Get the list of completion strings at specific location.

### `libclang#deduction#comment_at({filename}, {line}, {col} [, {compiler args}])`

Get brief comment for the entity referenced at a specific location.

### `libclang#deduction#declaration_at({filename}, {line}, {col} [, {compiler args}])`

Get location (file name, line, col) of the declaration referenced at a specific
location. This works not only for member functions, but for other entities like
local variables as well.

### `libclang#deduction#include_at({filename}, {line}, {col} [, {compiler args}])`

Get file name of the include referenced at a specific location.

### `libclang#deduction#diagnostics({filename}, [, {compiler args}])`

Get diagnostics (errors, warnings, etc) for a specific file.

### `libclang#deduction#compile_commands({filename})`

Get the list of compile commands for a specific file name.

## Installation

### LLVM Installation

- __OS X__

```
$ brew tap https://github.com/Homebrew/homebrew-dupes
$ brew update
$ brew install llvm34 --with-clang --with-libcxx --disable-assertions
```

- __Ubuntu__

```
$ sudo add-apt-repository --yes ppa:ubuntu-toolchain-r/test
$ wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | sudo apt-key add -
$ echo 'deb http://llvm.org/apt/precise/ llvm-toolchain-precise main' > llvm.list
$ echo 'deb-src http://llvm.org/apt/precise/ llvm-toolchain-precise main' >> llvm.list
$ sudo mv llvm.list /etc/apt/sources.list.d/
$ sudo apt-get update
$ sudo apt-get install llvm-3.5 clang-3.5 libclang-3.5-dev
```

- __Other OS__

Please lookup how to install the latest stable LLVM manually.

### libclang-vim Installation

- __With `Makefile`__

`llvm-config` command is required.  After cloning this repository, execute `make` in the repository.

```
$ cd libclang-vim/
$ ./autogen.sh
$ make
```

Or compile `lib/libclang-vim/clang_vim.cpp` manually as a shared object.

## Environment

I check libclang-vim in below environment.  It may not work in other environments.
If you see some errors, issues or pull requests are welcome.

- OS X 10.9, LLVM 3.4 (installed with Homebrew)
- Ubuntu 12.04, LLVM 3.5 (installed with apt)
- BGM: [TSAR MOMBA](http://www.youtube.com/watch?v=wi4WRhwhnCk)

## Vim Plugins Using libclang-vim

- [vim-textobj-clang](https://github.com/rhysd/vim-textobj-clang) : Deal with many C++ structures as text objects.
- [clang-extent-selector.vim](https://github.com/rhysd/clang-extent-selector.vim) : Only one mapping to select various C and C++ blocks.
- [clang-type-inspector.vim](https://github.com/rhysd/clang-type-inspector.vim) : Show type under the cursor and mouse pointer even if it is `auto`.
- [vim-textobj-function-clang](https://github.com/rhysd/vim-textobj-function-clang) : a plugin of vim-textobj-function.

## Example

### Get AST Information

- __Input file__

```cpp
#include <iostream>

int main()
{
    std::cout << "Hello, libclang!\n" << std::endl;

    return 0;
}
```

- __Vim command__

```
:call libclang#AST#non_system_headers#all('hello_libclang.cpp')
```

- __Result dictionary__

```
{
    'root': [
        {
            'USR': 'c:@F@main#',
            'children': [
                {
                    'children': [
                        {
                            'children': [
                                {
                                    'children': [
                                        {
                                            'children': [{'children': [], 'column': '5', 'file': 'tmp.cpp', 'kind': 'NamespaceRef', 'kind_type': 'Reference', 'line': '5', 'offset': '38', 'parent': 'cout', 'spell': 'std', 'type_kind': 'Invalid'}],
                                            'column': '10',
                                            'file': 'tmp.cpp',
                                            'kind': 'DeclRefExpr',
                                            'kind_type': 'Expression',
                                            'line': '5',
                                            'offset': '43',
                                            'parent': 'operator<<',
                                            'spell': 'cout',
                                            'type': 'ostream',
                                            'type_kind': 'Typedef'
                                        },
                                        {
                                            'children': [
                                                {
                                                    'children': [],
                                                    'column': '15',
                                                    'file': 'tmp.cpp',
                                                    'kind': 'DeclRefExpr',
                                                    'kind_type': 'Expression',
                                                    'line': '5',
                                                    'offset': '48',
                                                    'parent': 'operator<<',
                                                    'spell': 'operator<<',
                                                    'type': 'basic_ostream<char, std::__1::char_traits<char> > &(basic_ostream<char, std::__1::char_traits<char> > &, const char *)',
                                                    'type_kind': 'FunctionProto'
                                                }
                                            ],
                                            'column': '15',
                                            'file': 'tmp.cpp',
                                            'is_POD_type': 1,
                                            'kind': 'UnexposedExpr',
                                            'kind_type': 'Expression',
                                            'line': '5',
                                            'offset': '48',
                                            'parent': 'operator<<',
                                            'spell': 'operator<<',
                                            'type': 'basic_ostream<char, std::__1::char_traits<char> > &(*)(basic_ostream<char, std::__1::char_traits<char> > &, const char *)',
                                            'type_kind': 'Pointer'
                                        },
                                        {
                                            'children': [{'children': [], 'column': '18', 'file': 'tmp.cpp', 'is_POD_type': 1, 'kind': 'StringLiteral', 'kind_type': 'Expression', 'line': '5', 'offset': '51', 'type': 'const char [18]', 'type_kind': 'ConstantArray'}],
                                            'column': '18',
                                            'file': 'tmp.cpp',
                                            'is_POD_type': 1,
                                            'kind': 'UnexposedExpr',
                                            'kind_type': 'Expression',
                                            'line': '5',
                                            'offset': '51',
                                            'parent': 'operator<<',
                                            'type': 'const char *',
                                            'type_kind': 'Pointer'
                                        }
                                    ],
                                    'column': '5',
                                    'file': 'tmp.cpp',
                                    'kind': 'CallExpr',
                                    'kind_type': 'Expression',
                                    'line': '5',
                                    'offset': '38',
                                    'parent': 'operator<<',
                                    'spell': 'operator<<',
                                    'type': 'basic_ostream<char, std::__1::char_traits<char> >',
                                    'type_kind': 'Unexposed'
                                },
                                {
                                    'children': [
                                        {
                                            'children': [],
                                            'column': '39',
                                            'file': 'tmp.cpp',
                                            'kind': 'DeclRefExpr',
                                            'kind_type': 'Expression',
                                            'line': '5',
                                            'offset': '72',
                                            'parent': 'operator<<',
                                            'spell': 'operator<<',
                                            'type': 'std::__1::basic_ostream<char, std::__1::char_traits<char> > &(std::__1::basic_ostream<char, std::__1::char_traits<char> > &(*)(std::__1::basic_ostream<char, std::__1::char_traits<char> > &))',
                                            'type_kind': 'FunctionProto'
                                        }
                                    ],
                                    'column': '39',
                                    'file': 'tmp.cpp',
                                    'is_POD_type': 1,
                                    'kind': 'UnexposedExpr',
                                    'kind_type': 'Expression',
                                    'line': '5',
                                    'offset': '72',
                                    'parent': 'operator<<',
                                    'spell': 'operator<<',
                                    'type': 'std::__1::basic_ostream<char, std::__1::char_traits<char> > &(*)(std::__1::basic_ostream<char, std::__1::char_traits<char> > &(*)(std::__1::basic_ostream<char, std::__1::char_traits<char> > &))',
                                    'type_kind': 'Pointer'
                                },
                                {
                                    'children': [
                                        {
                                            'children': [{'children': [], 'column': '42', 'file': 'tmp.cpp', 'kind': 'NamespaceRef', 'kind_type': 'Reference', 'line': '5', 'offset': '75', 'parent': 'endl', 'spell': 'std', 'type_kind': 'Invalid'}],
                                            'column': '47',
                                            'file': 'tmp.cpp',
                                            'kind': 'DeclRefExpr',
                                            'kind_type': 'Expression',
                                            'line': '5',
                                            'offset': '80',
                                            'parent': 'endl',
                                            'spell': 'endl',
                                            'type': 'basic_ostream<char, std::__1::char_traits<char> > &(basic_ostream<char, std::__1::char_traits<char> > &)',
                                            'type_kind': 'FunctionProto'
                                        }
                                    ],
                                    'column': '47',
                                    'file': 'tmp.cpp',
                                    'is_POD_type': 1,
                                    'kind': 'UnexposedExpr',
                                    'kind_type': 'Expression',
                                    'line': '5',
                                    'offset': '80',
                                    'parent': 'operator<<',
                                    'spell': 'endl',
                                    'type': 'basic_ostream<char, std::__1::char_traits<char> > &(*)(basic_ostream<char, std::__1::char_traits<char> > &)',
                                    'type_kind': 'Pointer'
                                }
                            ],
                            'column': '5',
                            'file': 'tmp.cpp',
                            'kind': 'CallExpr',
                            'kind_type': 'Expression',
                            'line': '5',
                            'offset': '38',
                            'spell': 'operator<<',
                            'type': 'std::__1::basic_ostream<char, std::__1::char_traits<char> >',
                            'type_kind': 'Record'
                        },
                        {
                            'children': [{'children': [], 'column': '12', 'file': 'tmp.cpp', 'is_POD_type': 1, 'kind': 'IntegerLiteral', 'kind_type': 'Expression', 'line': '7', 'offset': '98', 'type': 'int', 'type_kind': 'Int'}],
                            'column': '5',
                            'file': 'tmp.cpp',
                            'kind': 'ReturnStmt',
                            'kind_type': 'Statement',
                            'line': '7',
                            'offset': '91',
                            'type_kind': 'Invalid'
                        }
                    ],
                    'column': '1',
                    'file': 'tmp.cpp',
                    'kind': 'CompoundStmt',
                    'kind_type': 'Statement',
                    'line': '4',
                    'offset': '32',
                    'parent': 'main',
                    'semantic_parent': 'main',
                    'type_kind': 'Invalid'
                }
            ],
            'column': '5',
            'file': 'tmp.cpp',
            'is_definition': 1,
            'kind': 'FunctionDecl',
            'kind_type': 'Declaration',
            'lexical_parent': 'tmp.cpp',
            'line': '3',
            'linkage': 'External',
            'offset': '25',
            'parent': 'tmp.cpp',
            'semantic_parent': 'tmp.cpp',
            'spell': 'main',
            'type': 'int ()',
            'type_kind': 'FunctionProto'
        }
    ]
}
```

## Trouble Shootings

### "fatal error: 'bits/c++config.h' file not found" occurs

Include a path to `bits/c++config.h` in `CXXFLAGS`.  Below is an example in Ubuntu 12.04

```
CXXFLAGS=-I/usr/include/i386-linux-gnu/c++/4.8 make
```

## License

    The MIT License (MIT)

    Copyright (c) 2014 rhysd

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

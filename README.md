Vim script Wrapper for Libclang
===============================

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
- get definition, decralation and refarenced node of the item at specific location
- get pointee type, result type, canonical type (unaliased type aliased by typedef)
- get the information for completion. (not implemented yet)
- get diagnostic information. (not implemented yet)
- get preprocessing information. (not implemented yet)
- get comment information. (not implemented yet)


## Usage

### `libclang#version()`

Get version of libclang as a string.

### `libclang#tokens#all({filename})`

Get tokens in `{filename}`.  It includes all tokens in included header files.

### `libclang#AST#{extent}#{kind of node}({filename})`

Get information of specific kind of node in AST as a dictionary.

`{extent}` is the extent of analysis. `whole` searches all of the code, `non_system_headers` searches all of the code except for system headers (which are included with `#include <>`), `current_file` searches the code in only the current file.

`{kind of node}` is a kind of AST nodes which you want to extract.  `all` extracts all kind of AST nodes, `declarations` extracts all AST nodes related to declarations, `definitions` extracts all AST nodes related to definitions, `expressions` extracts all AST node related to expressions, and so on.

If you want to get information about definitions and not to get AST information about system headers, you should use `libclang#AST#non_system_headers#definitions()`.

### `libclang#location#AST_node({filename}, {line}, {col})`

Get the AST node information at specific location.

### `libclang#location#extent({filename}, {line}, {col})`

Get the extent of the most inner syntax element at specific location.

### `libclang#location#{syntax element}_extent({filename}, {line}, {col})`

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

### `libclang#location#{something}_at({filename}, {line}, {col})`

Get the node information related at specific location.

`{something}` is one of below items.

- `definition` : definition node of node at specific location
- `referenced` : node which node at specific location references
- `declaration` : declaration node of node at specific location
- `pointee_type` : type of the pointer at specific location
- `canonical_type` : canonical type of the type at specific location
- `result_type` : result type at specific location (e.g. result type of function)
- `class_type_of_member_pointer` : class type of member funciton at specific location

If you want to get the definition of specific location, you should use `libclang#location#definition_at()`.
If you want to know what item specific location references, you should use `libclang#location#referenced_at()`.
If you want to get the type of a function at specific location, you should use `libclang#locaiton#result_type_at()`.

## Installation

`llvm-config` command is required.  After cloning this repository, execute `make` in the repository.  Or compile `lib/clang_vim.cpp` manually.


## Environment

I check libclang-vim in below environment.  It may not work in other environments.
If you see some errors, issues or pull requests are welcome.

- OS X 10.9, LLVM 3.4 (installed with Homebrew)
- Ubuntu 12.04, LLVM 3.5 (installed with apt)
- BGM: [TSAR MOMBA](http://www.youtube.com/watch?v=wi4WRhwhnCk)

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

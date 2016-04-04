include config.mak

TARGET=lib/libclang-vim.so
SRC=lib/libclang-vim/clang_vim.cpp lib/libclang-vim/AST_extracter.hpp lib/libclang-vim/helpers.hpp lib/libclang-vim/location.hpp lib/libclang-vim/stringizers.hpp lib/libclang-vim/tokenizer.hpp lib/libclang-vim/deduction.hpp
CPPSRC=lib/libclang-vim/clang_vim.cpp
CXXFLAGS+=$(shell $(LLVMCONFIG) --cxxflags --ldflags) -Wall -Wextra -std=c++11 -pedantic -shared -fPIC -lclang

# For LLVM installed in a custom location
LDFLAGS+=-rpath $(shell $(LLVMCONFIG) --libdir)

all: $(TARGET)

config.mak: configure.ac config.mak.in qa/data/compile-commands/compile_commands.json.in
	./autogen.sh

$(TARGET): $(SRC) config.mak
	$(CLANG) $(CXXFLAGS) $(CPPSRC) $(LDFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)

qa/test: qa/test.cpp qa/deduction.cpp config.mak
	$(CLANG) -std=c++11 $(CPPUNIT_CFLAGS) qa/test.cpp qa/deduction.cpp $(LDFLAGS) $(CPPUNIT_LIBS) -ldl -o qa/test

check: qa/test $(TARGET)
	qa/test

tags:
	ctags --c++-kinds=+p --fields=+iaS --extra=+q -R --totals=yes *

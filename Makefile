include config.mak

TARGET=lib/libclang-vim.so
SRC=lib/libclang-vim/clang_vim.cpp lib/libclang-vim/AST_extracter.hpp lib/libclang-vim/helpers.hpp lib/libclang-vim/location.hpp lib/libclang-vim/stringizers.hpp lib/libclang-vim/tokenizer.hpp lib/libclang-vim/deduction.hpp
CPPSRC=lib/libclang-vim/clang_vim.cpp
CXXFLAGS+=-Wall -Wextra -std=c++11 -pedantic -fPIC

# For LLVM installed in a custom location
LDFLAGS+=-rpath $(LLVM_LIBDIR)

all: $(TARGET)

config.mak: configure.ac config.mak.in qa/data/compile-commands/compile_commands.json.in
	./autogen.sh

$(TARGET): $(SRC) config.mak
	$(CXX) $(CXXFLAGS) $(LLVM_CXXFLAGS) $(CPPSRC) $(LDFLAGS) $(LLVM_LDFLAGS) -lclang -shared -o $(TARGET)

clean:
	rm -f $(TARGET)

qa/test: qa/test.cpp qa/deduction.cpp config.mak
	$(CXX) $(CXXFLAGS) $(CPPUNIT_CFLAGS) qa/test.cpp qa/deduction.cpp $(CPPUNIT_LIBS) -ldl -o qa/test

check: qa/test $(TARGET)
	qa/test

tags:
	ctags --c++-kinds=+p --fields=+iaS --extra=+q -R --totals=yes *

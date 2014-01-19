ifeq ($(shell which llvm-config-3.4),)
ifeq ($(shell which llvm-config-3.5),)
LLVMCONFIG=llvm-config
else
LLVMCONFIG=llvm-config-3.5
endif
else
LLVMCONFIG=llvm-config-3.4
endif

TARGET=lib/libclang-vim.so
SRC=lib/libclang-vim/clang_vim.cpp lib/libclang-vim/AST_extracter.hpp lib/libclang-vim/helpers.hpp lib/libclang-vim/location.hpp lib/libclang-vim/stringizers.hpp lib/libclang-vim/tokenizer.hpp
CPPSRC=lib/libclang-vim/clang_vim.cpp
CXXFLAGS=$(shell $(LLVMCONFIG) --cxxflags --ldflags) -Wall -Wextra -std=c++11 -pedantic -shared -fPIC -lclang

# For LLVM installed by Homebrew
UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
LDFLAGS+=-rpath $(shell $(LLVMCONFIG) --libdir)
endif

all: $(TARGET)

$(TARGET): $(SRC)
	clang++ $(CXXFLAGS) $(CPPSRC) $(LDFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)

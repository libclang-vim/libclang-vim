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
SRC=lib/clang_vim.cpp
CXXFLAGS=$(shell $(LLVMCONFIG) --cxxflags --ldflags) -Wall -Wextra -std=c++11 -pedantic -shared -fPIC -lclang

# For LLVM installed by Homebrew
UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
LDFLAGS+=-rpath $(shell $(LLVMCONFIG) --libdir)
endif

all: $(TARGET)

$(TARGET): $(SRC)
	clang++ $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)

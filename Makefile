CXXFLAGS=$(shell llvm-config-3.4 --cxxflags --ldflags) -Wall -Wextra -std=c++11 -pedantic -shared

TARGET=lib/libclang-vim.so
SRC=lib/libclang_vim.cpp
CXXFLAGS+=-lclang -fPIC
LDFLAGS+=-rpath $(shell llvm-config-3.4 --libdir)

all: $(TARGET)

$(TARGET): $(SRC)
	clang++ $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)

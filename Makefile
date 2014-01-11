TARGET=lib/libclang-vim.so
SRC=lib/libclang_vim.cpp
CXXFLAGS=$(shell llvm-config-3.4 --cxxflags --ldflags) -Wall -Wextra -std=c++11 -pedantic -shared -fPIC -lclang
CXXFLAGS+=-lclang -fPIC
UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
	LDFLAGS+=-rpath $(shell llvm-config-3.4 --libdir)
endif

all: $(TARGET)

$(TARGET): $(SRC)
	clang++ $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)

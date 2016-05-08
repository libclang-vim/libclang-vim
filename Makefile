include config.mak
CXXFLAGS+=-Wall -Wextra -std=c++11 -pedantic -fPIC
# For LLVM installed in a custom location
LDFLAGS+=-rpath $(LLVM_LIBDIR)

DEPDIR := .d
COMPILE.cc = $(CXX) $(CXXFLAGS) -c

all: lib/libclang-vim.so qa/test qa/tool

lib_objects = lib/libclang-vim/clang_vim.o lib/libclang-vim/deduction.o
lib/libclang-vim.so: $(lib_objects)
	$(LINK.cpp) $^ $(LDFLAGS) $(LLVM_LDFLAGS) -lclang -shared -o $@

qa_objects = qa/test.o qa/deduction.o
qa/test: $(qa_objects)
	$(LINK.cpp) $^ $(CPPUNIT_LIBS) -ldl -o $@

tool_objects = qa/tool.o
qa/tool: $(tool_objects)
	$(LINK.cpp) $^ -ldl -o $@

all_objects = $(lib_objects) $(qa_objects) $(tool_objects)

lib/libclang-vim/%.o : lib/libclang-vim/%.cpp
	mkdir -p $(DEPDIR)/lib/libclang-vim
	$(COMPILE.cc) -MT $@ -MMD -MP -MF $(DEPDIR)/lib/libclang-vim/$*.d_ $(LLVM_CXXFLAGS) $(OUTPUT_OPTION) $<
	mv $(DEPDIR)/lib/libclang-vim/$*.d_ $(DEPDIR)/lib/libclang-vim/$*.d

qa/%.o : qa/%.cpp
	mkdir -p $(DEPDIR)/qa
	$(COMPILE.cc) -MT $@ -MMD -MP -MF $(DEPDIR)/qa/$*.d_ $(CPPUNIT_CFLAGS) -DSRC_ROOT=\"$(SRC_ROOT)\" $(OUTPUT_OPTION) $<
	mv $(DEPDIR)/qa/$*.d_ $(DEPDIR)/qa/$*.d

SRCS = $(patsubst %.o,%.cpp,$(all_objects))
$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS)))

config.mak: configure.ac config.mak.in qa/data/compile-commands/compile_commands.json.in
	./autogen.sh

clean:
	rm -f lib/libclang-vim.so qa/test $(all_objects)

check: all
	qa/test

tags:
	ctags --c++-kinds=+p --fields=+iaS --extra=+q -R --totals=yes *

clang-tidy-modernize:
	clang-tidy -checks=*modernize*,-modernize-raw-string-literal -header-filter=.* $(SRCS)

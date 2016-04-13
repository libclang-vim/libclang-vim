AC_INIT(libclang-vim, 1.0)

AC_ARG_ENABLE(tests,
    AS_HELP_STRING([--disable-tests],
        [Build and run unit tests.]),
,enable_tests=yes)

AC_ARG_ENABLE(werror,
    AS_HELP_STRING([--enable-werror],
        [Turn warnings to errors. (Has no effect in modules where the treating
         of warnings as errors is disabled explicitly.)]),
,)

AC_CHECK_PROGS(LLVMCONFIG, llvm-config llvm-config-3.5 llvm-config-3.4)
if test -z "$LLVMCONFIG"; then
    AC_MSG_ERROR([llvm-config not found])
fi
LLVM_CXXFLAGS=`$LLVMCONFIG --cxxflags`
AC_SUBST(LLVM_CXXFLAGS)
LLVM_LDFLAGS=`$LLVMCONFIG --ldflags`
AC_SUBST(LLVM_LDFLAGS)
LLVM_LIBDIR=`$LLVMCONFIG --libdir`
AC_SUBST(LLVM_LIBDIR)

AC_CHECK_PROGS(CXX, clang++ clang++-3.5 clang++-3.4)
if test -z "$CXX"; then
    AC_MSG_ERROR([clang++ not found])
fi
AC_SUBST(CXX)

if test "$enable_tests" != "no"; then
    PKG_CHECK_MODULES([CPPUNIT], [cppunit])
fi
AC_SUBST([CPPUNIT_CFLAGS])
AC_SUBST([CPPUNIT_LIBS])

AC_MSG_CHECKING([whether to turn warnings to errors])
if test -n "$enable_werror" -a "$enable_werror" != "no"; then
    CXXFLAGS="$CXXFLAGS -Werror"
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(CXXFLAGS)

SRC_ROOT=`pwd`
AC_SUBST(SRC_ROOT)

AC_CONFIG_FILES([config.mak
                 qa/data/compile-commands/compile_commands.json])
AC_OUTPUT

dnl vim:set shiftwidth=4 softtabstop=4 expandtab:

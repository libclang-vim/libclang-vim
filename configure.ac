AC_INIT(libclang-vim, 1.0)

AC_ARG_ENABLE(tests,
    AS_HELP_STRING([--disable-tests],
        [Build and run unit tests.]),
,enable_tests=yes)

AC_CHECK_PROGS(LLVMCONFIG, llvm-config llvm-config-3.5 llvm-config-3.4)
if test -z "$LLVMCONFIG"; then
    AC_MSG_ERROR([llvm-config not found])
fi
AC_SUBST(LLVMCONFIG)

AC_CHECK_PROGS(CLANG, clang++ clang++-3.5 clang++-3.4)
if test -z "$CLANG"; then
    AC_MSG_ERROR([clang++ not found])
fi
AC_SUBST(CLANG)

if test "$enable_tests" != "no"; then
    PKG_CHECK_MODULES([CPPUNIT], [cppunit])
fi
AC_SUBST([CPPUNIT_CFLAGS])
AC_SUBST([CPPUNIT_LIBS])

AC_OUTPUT(config.mak)

dnl vim:set shiftwidth=4 softtabstop=4 expandtab:

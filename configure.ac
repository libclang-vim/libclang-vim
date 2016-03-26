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

AC_MSG_CHECKING([whether to turn warnings to errors])
if test -n "$enable_werror" -a "$enable_werror" != "no"; then
    CXXFLAGS="$CXXFLAGS -Werror"
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(CXXFLAGS)

AC_OUTPUT(config.mak)

dnl vim:set shiftwidth=4 softtabstop=4 expandtab:

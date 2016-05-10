AC_INIT(libclang-vim, 1.0)

AC_ARG_ENABLE(tests,
    AS_HELP_STRING([--disable-tests],
        [Build and run unit tests.]),
,enable_tests=yes)

AC_ARG_ENABLE(werror,
    AS_HELP_STRING([--enable-werror],
        [Handle warnings as errors.]),
,)
AC_ARG_ENABLE(debug,
    AS_HELP_STRING([--enable-debug],
        [Include debugging information, disable compiler optimization and
         enable assertions.]),
,)

AC_CHECK_PROGS(LLVMCONFIG, llvm-config llvm-config-3.5 llvm-config-3.4)
if test -z "$LLVMCONFIG"; then
    AC_MSG_ERROR([llvm-config not found])
fi
LLVM_CXXFLAGS=`$LLVMCONFIG --cxxflags|sed 's/-O. //;s/-g //;s/-DNDEBUG //'`
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

AC_MSG_CHECKING([whether to enable debugging])
if test -n "$enable_debug" -a "$enable_debug" != "no"; then
    CXXFLAGS="$CXXFLAGS -O0 -g"
    AC_MSG_RESULT([yes])
else
    if test -z "$CXXFLAGS"; then
        CXXFLAGS="-O2"
    fi
    CXXFLAGS="$CXXFLAGS -DNDEBUG"
    AC_MSG_RESULT([no])
fi

AC_MSG_CHECKING([whether to handle warnings as errors])
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

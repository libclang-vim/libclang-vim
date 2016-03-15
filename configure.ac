AC_INIT(libclang-vim, 1.0)

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

AC_OUTPUT(config.mak)

dnl vim:set shiftwidth=4 softtabstop=4 expandtab:

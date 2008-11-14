dnl ---------------------------------------------------------------------------
dnl Macro: 64BIT
dnl ---------------------------------------------------------------------------
AC_ARG_ENABLE(64bit,
    [  --enable-64bit      Build 64bit library.],
    [ 
       org_cflags=$CFLAGS
       CFLAGS=-m64
       AC_LANG(C)
       AC_RUN_IFELSE([
            AC_LANG_PROGRAM([], [ if (sizeof(void*) != 8) return 1;])
          ],[
            CFLAGS="$CFLAGS $org_cflags"
          ],[
            AC_MSG_ERROR([Don't know how to build a 64-bit object.])
          ])
       org_cxxflags=$CXXFLAGS
       CXXFLAGS=-m64
       AC_LANG(C++)
       AC_RUN_IFELSE([
            AC_LANG_PROGRAM([], [ if (sizeof(void*) != 8) return 1;])
          ],[
            CXXFLAGS="$CXXFLAGS $org_cxxflags"
          ],[
            AC_MSG_ERROR([Don't know how to build a 64-bit object.])
          ])

    ])
dnl ---------------------------------------------------------------------------
dnl End Macro: 64BIT
dnl ---------------------------------------------------------------------------

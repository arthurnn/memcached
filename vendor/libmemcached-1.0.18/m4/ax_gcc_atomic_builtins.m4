# serial 1
#
AC_DEFUN([HAVE_GCC_ATOMICS],
    [AC_CACHE_CHECK([whether the compiler provides atomic builtins],
      [ax_cv_gcc_atomic_builtins],
      [AX_SAVE_FLAGS
      AC_LANG_PUSH([C])
      AC_RUN_IFELSE([AC_LANG_PROGRAM([],
          [[
          int foo= -10; int bar= 10;
          if (!__sync_fetch_and_add(&foo, bar) || foo)
          {
          return -1;
          }
          bar= __sync_lock_test_and_set(&foo, bar);
          if (bar || foo != 10)
          {
          return -1;
          }
          bar= __sync_val_compare_and_swap(&bar, foo, 15);
          if (bar)
          {
          return -1;
          }
          ]])],
        [ax_cv_gcc_atomic_builtins=yes],
        [ax_cv_gcc_atomic_builtins=no],
        [AC_MSG_WARN([test program execution failed])])
      AC_LANG_POP
      AX_RESTORE_FLAGS
      ])

      AS_IF([test "x$ax_cv_gcc_atomic_builtins" = "xyes"],
          [AC_DEFINE([HAVE_GCC_ATOMIC_BUILTINS],[1],
            [Define to 1 if compiler provides atomic builtins.])
          ])
      ])

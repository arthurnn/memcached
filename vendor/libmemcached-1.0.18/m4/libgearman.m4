# serial 2
AC_DEFUN([CHECK_FOR_LIBGEARMAND],
         [AX_CHECK_LIBRARY([LIBGEARMAN],[libgearman/gearman.h],[gearman],,
                           [AC_DEFINE([HAVE_LIBGEARMAN],[0],[Define to 1 to compile in libgearman support])])
         ])

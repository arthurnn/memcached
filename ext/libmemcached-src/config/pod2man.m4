AC_PATH_PROG([POD2MAN], [pod2man], "no", [$PATH:/usr/bin:/usr/local/bin])
if test "x$POD2MAN" == "xno"; then
  AC_MSG_ERROR(["Could not find pod2man anywhere in path"])
fi
AC_SUBST(POD2MAN)

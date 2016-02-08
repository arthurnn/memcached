dnl ---------------------------------------------------------------------------
dnl Macro: SETSOCKOPT_SANITY
dnl ---------------------------------------------------------------------------
AC_DEFUN([SETSOCKOPT_SANITY],[
  AC_LANG_PUSH([C])
  AC_RUN_IFELSE([ 
    AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
   ]],[[
     int sock = socket(AF_INET, SOCK_STREAM, 0);
     struct timeval waittime;
   
     waittime.tv_sec= 0;
     waittime.tv_usec= 500;
   
     if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, 
                    &waittime, (socklen_t)sizeof(struct timeval)) == -1) {
       if (errno == ENOPROTOOPT) {
         return 1;
       }
     }
     return 0;
   ]])],
   [AC_DEFINE(HAVE_SNDTIMEO, 1, [Define to 1 if you have a working SO_SNDTIMEO])]) 

  AC_RUN_IFELSE([ 
    AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
    ]],[[
     int sock = socket(AF_INET, SOCK_STREAM, 0);
     struct timeval waittime;
   
     waittime.tv_sec= 0;
     waittime.tv_usec= 500;
   
     if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, 
                    &waittime, (socklen_t)sizeof(struct timeval)) == -1) {
       if (errno == ENOPROTOOPT) {
         return 1;
       }
     }
     return 0;
   ]])], [AC_DEFINE(HAVE_RCVTIMEO, 1, [Define to 1 if you have a working SO_RCVTIMEO])]) 

  AC_LANG_POP
])
dnl ---------------------------------------------------------------------------
dnl End Macro: SETSOCKOPT_SANITY
dnl ---------------------------------------------------------------------------

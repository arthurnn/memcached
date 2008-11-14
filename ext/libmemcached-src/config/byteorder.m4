AC_RUN_IFELSE([ 
   AC_LANG_PROGRAM([
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
      ], [
if (htonl(5) != 5) {
   return 1;
}
      ])            
   ], AC_DEFINE([BYTEORDER_BIG_ENDIAN], [1], [Enable big endian byteorder]))

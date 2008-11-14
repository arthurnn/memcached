/*
 * Summary: Localized copy of WATCHPOINT debug symbols
 *
 * Copy: See Copyright for the status of this software.
 *
 * Author: Brian Aker
 */

#ifndef __MEMCACHED_WATCHPOINT_H__
#define __MEMCACHED_WATCHPOINT_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Some personal debugging functions */
#if defined(MEMCACHED_INTERNAL) && defined(HAVE_DEBUG)
#include <assert.h>

#define WATCHPOINT fprintf(stderr, "\nWATCHPOINT %s:%d (%s)\n", __FILE__, __LINE__,__func__);fflush(stdout);
#define WATCHPOINT_ERROR(A) fprintf(stderr, "\nWATCHPOINT %s:%d %s\n", __FILE__, __LINE__, memcached_strerror(NULL, A));fflush(stdout);
#define WATCHPOINT_IFERROR(A) if(A != MEMCACHED_SUCCESS)fprintf(stderr, "\nWATCHPOINT %s:%d %s\n", __FILE__, __LINE__, memcached_strerror(NULL, A));fflush(stdout);
#define WATCHPOINT_STRING(A) fprintf(stderr, "\nWATCHPOINT %s:%d (%s) %s\n", __FILE__, __LINE__,__func__,A);fflush(stdout);
#define WATCHPOINT_STRING_LENGTH(A,B) fprintf(stderr, "\nWATCHPOINT %s:%d (%s) %.*s\n", __FILE__, __LINE__,__func__,(int)B,A);fflush(stdout);
#define WATCHPOINT_NUMBER(A) fprintf(stderr, "\nWATCHPOINT %s:%d (%s) %zu\n", __FILE__, __LINE__,__func__,(size_t)(A));fflush(stdout);
#define WATCHPOINT_ERRNO(A) fprintf(stderr, "\nWATCHPOINT %s:%d (%s) %s\n", __FILE__, __LINE__,__func__, strerror(A));fflush(stdout);
#define WATCHPOINT_ASSERT_PRINT(A,B,C) if(!(A)){fprintf(stderr, "\nWATCHPOINT ASSERT %s:%d (%s) ", __FILE__, __LINE__,__func__);fprintf(stderr, (B),(C));fprintf(stderr,"\n");fflush(stdout);}assert((A));
#define WATCHPOINT_ASSERT(A) assert((A));
#else
#define WATCHPOINT
#define WATCHPOINT_ERROR(A)
#define WATCHPOINT_IFERROR(A)
#define WATCHPOINT_STRING(A)
#define WATCHPOINT_NUMBER(A)
#define WATCHPOINT_ERRNO(A)
#define WATCHPOINT_ASSERT_PRINT(A,B,C)
#define WATCHPOINT_ASSERT(A)

#endif /* MEMCACHED_INTERNAL && HAVE_DEBUG */

#ifdef __cplusplus
}
#endif

#endif /* __MEMCACHED_WATCHPOINT_H__ */

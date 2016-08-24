#ifndef jjp_dbg_h
#define jjp_dbg_h


#ifdef JJP_LOG

#include <stdio.h>
#include <errno.h>
#include <string.h>

#define clean_errno() (errno==0 ? "None" : strerror(errno))

#define log_err_empty() fprintf(stderr, "[ERROR] (%s:%d: errno: %s)\n", __FILE__, __LINE__, clean_errno())

#define check(A, B) if(!(A)) { log_err_empty(); errno=0; goto B; }

#else

#define check(A, B) if(!(A)) { goto B; }

#endif

#endif

#ifndef jjp_jsonpath_h
#define jjp_jsonpath_h

#include <jsmn.h>




#define JJP_DEFAULT_ALLOCATED_RESULT_TOKENS 16
#define JJP_MAX_JSONPATH_LENGTH 10000

typedef struct {
	unsigned int count;
	unsigned int * tokens;
} jjp_result_t;

jjp_result_t * jjp_jsonpath(
		const char * json,
		jsmntok_t * tokens,
		unsigned int tokens_count,
		const char * jsonpath,
		unsigned int current_object
		);

void jjp_result_destroy( jjp_result_t * result );

int jjp_jsonpath_first(
		const char * json,
		jsmntok_t * tokens,
		unsigned int tokens_count,
		const char * jsonpath,
		unsigned int current_object
		);

#endif

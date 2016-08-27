#ifndef jjp_jsonpath_h
#define jjp_jsonpath_h

#include <jsmn.h>




#define JJP_DEFAULT_ALLOCATED_RESULT_TOKENS 16
#define JJP_MAX_JSONPATH_LENGTH 10000

typedef enum {
	/* No error occured */
	JJP_OK = 0,
	/* Current or first token is not an object */
	JJP_ERR_NOOBJ = -2,
	/* Unsupported JSONPath expression */
	JJP_ERR_UNSUPPORTED = -3,
	/* JSONPath expression to long or no null terminator at end */
	JJP_ERR_TO_LONG = -4,
	/* Out of heap memory (realloc call failed) */
	JJP_ERR_OUT_OF_MEMORY = -5,
	/* The json was weird or we have a bug... */
	JJP_ERR_WEIRD_JSON = -6
} jjp_err_t;

typedef struct {
	jjp_err_t error;
	unsigned int count;
	unsigned int * tokens;
} jjp_result_t;

jjp_err_t jjp_jsonpath(
		const char * json,
		jsmntok_t * tokens,
		unsigned int tokens_count,
		const char * jsonpath,
		unsigned int current_object,
		jjp_result_t * result
		);

jjp_err_t jjp_jsonpath_first(
		const char * json,
		jsmntok_t * tokens,
		unsigned int tokens_count,
		const char * jsonpath,
		unsigned int current_object,
		unsigned int * result
		);

void jjp_result_deinit( jjp_result_t * result );

#endif

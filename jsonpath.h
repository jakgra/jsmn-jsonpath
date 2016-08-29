#ifndef jjp_jsonpath_h
#define jjp_jsonpath_h

#include <jsmn.h>




#define JJP_DEFAULT_ALLOCATED_RESULT_TOKENS 16
#define JJP_MAX_JSONPATH_LENGTH 10000

/**
 * jsmn-jsonpath result type.
 */
typedef struct {
	unsigned int count; /**< The count of found elements in the json that match the jsonpath. */
	unsigned int * tokens; /**< A pointer to an int[] with count elements or NULL if count == 0. */
} jjp_result_t;

/**
 * \Return NULL on error or a pointer to a jjp_result_t that was allocated on the stack using malloc.
 * You must free the memory used by the returned object after using it, by calling jjp_result_destroy().
 */
jjp_result_t * jjp_jsonpath(
		const char * json,
		jsmntok_t * tokens,
		unsigned int tokens_count,
		const char * jsonpath,
		unsigned int current_object
		);

/**
 * Frees the memory used by result
 */
void jjp_result_destroy( jjp_result_t * result );

/**
 * \Returns -1 on error or the index of the first token that matches the jsonpath.
 */
int jjp_jsonpath_first(
		const char * json,
		jsmntok_t * tokens,
		unsigned int tokens_count,
		const char * jsonpath,
		unsigned int current_object
		);

#endif

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
 * \Returns a pointer to a jjp_result_t that was allocated on the stack using malloc or NULL on error.
 * You must free the memory used by the returned object after using it, by calling jjp_result_destroy().
 */
jjp_result_t * jjp_jsonpath(
		const char * json, /**< The js parameter from jsmn_parse() */
		jsmntok_t * tokens, /**< The tokens parameter from jsmn_parse() */
		unsigned int tokens_count, /**< The return value from jsmn_parse() */
		const char * jsonpath, /**< The JSONPath you want to find matches for */
		unsigned int current_object /**< The index of the current element inside the tokens array */
		);

/**
 * Frees the memory used by result
 */
void jjp_result_destroy( jjp_result_t * result );

/**
 * \Returns the index of the first token that matches the jsonpath or -1 if no matches were found or -2 on error.
 */
int jjp_jsonpath_first(
		const char * json, /**< see jjp_jsonpath() */
		jsmntok_t * tokens, /**< see jjp_jsonpath() */
		unsigned int tokens_count, /**< see jjp_jsonpath() */
		const char * jsonpath, /**< see jjp_jsonpath() */
		unsigned int current_object /**< see jjp_jsonpath() */
		);

#endif

#include "jsonpath.h"
#ifndef JJP_NO_MALLOC
#include <stdlib.h>
#endif




// private functions

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
	int only_first;
#ifndef JJP_NO_MALLOC
	jjp_result_t * result;
#endif
	unsigned int max_mem;
	unsigned int count;
	int * provided_store;
} jjp_result_wrapper_t;

static char strings_are_equal( const char * s1, const char * s2, int len ) {

	int i;

	for( i = 0; i < len; i++ ) {
		if( s1[i] != s2[i] ) return 0;
	}

	return 1;

}

static void add_to_result( jjp_result_wrapper_t * wrap, int token ) {

#ifndef JJP_NO_MALLOC
	unsigned int * tmp;
#endif


	if( wrap->only_first == -1 ) {

		wrap->only_first = token;

	} else if( wrap->only_first == -3 ) {

		wrap->count++;
		if( wrap->count <= wrap->max_mem ) {
			wrap->provided_store[ wrap->count - 1 ] = token;
		} else {
			wrap->error = JJP_ERR_OUT_OF_MEMORY;
		}

	}
#ifndef JJP_NO_MALLOC
	else if( wrap->only_first == -2 ) {

		wrap->result->count++;

		if( ! wrap->result->tokens ) {
			wrap->result->tokens = malloc( JJP_DEFAULT_ALLOCATED_RESULT_TOKENS * sizeof( int ) );
			wrap->max_mem = JJP_DEFAULT_ALLOCATED_RESULT_TOKENS;
		}

		if( wrap->result->count > wrap->max_mem ) {

			tmp = realloc( wrap->result->tokens, wrap->max_mem * 2 * sizeof( int ) );
			check( tmp, final_cleanup );

			wrap->result->tokens = tmp;

		}

		wrap->result->tokens[ wrap->result->count - 1 ] = token;

	}
#endif

	return;

#ifndef JJP_NO_MALLOC
final_cleanup:
	if( wrap->result ) {
		free( wrap->result->tokens );
		wrap->result->tokens = NULL;
		wrap->result->count = 0;
		wrap->error = JJP_ERR_OUT_OF_MEMORY;
	}
	return;
#endif

}

static inline int get_next_point( const char * jsonpath, int start ) {

	char * cur_c;

	for( cur_c = (char *)jsonpath + start; cur_c - jsonpath < JJP_MAX_JSONPATH_LENGTH; cur_c++ ) {
		if( *cur_c == '.' || *cur_c == '[' || *cur_c == '\0' ) {
			break;
		}
	}
	check( cur_c - jsonpath < JJP_MAX_JSONPATH_LENGTH, final_cleanup );

	return cur_c - jsonpath;

final_cleanup:
	return JJP_ERR_TO_LONG;

}

static int to_int( char * a, int * len ) {

	const char * digits = "0123456789";
	int i;
	int j;
	int found;
	int result;
	int is_negative;


	is_negative = ( a[0] == '-' );

	result = 0;
	*len = 0;

	for( j = is_negative; j < 12; j++ ) {
		found = -1;
		for( i = 0; digits[i] != '\0'; i++ ) {
			if( a[j] == digits[i] ) {
				found = i;
				break;
			}
		}
		if( found == -1 ) {
			*len = j;
			break;
		} else {
			result *= 10;
			result += found;
		}
	}

	return ( is_negative ? -1 : 1 ) * result;

}

static void parse_recurse( const char * json, jsmntok_t * tok, unsigned int tok_count,
		const char * jsonpath, jjp_result_wrapper_t * wrap, int parent, int start ) {

	jjp_err_t rc;
	int end;
	unsigned int i;
	char is_wildcard;
	char recurse;


	if( *( jsonpath + start - 1 ) == '\0' ) {
		if( wrap->error == JJP_OK && wrap->only_first < 0 ) {
			add_to_result( wrap, parent );
		}
	} else if( *( jsonpath + start - 1 ) == '[' ) {

		if( tok[parent].type == JSMN_ARRAY ) {

			int index;
			int len;
			char arr_wildcard;


			arr_wildcard = ( *( jsonpath + start ) == '*' );
			if( arr_wildcard ) {
				len = 1;
			} else {
				index = to_int( (char *)jsonpath + start, &len );
			}
			check( len > 0 && *( jsonpath + start + len ) == ']'
					&& (
						*( jsonpath + start + len + 1 ) == '.'
						|| *( jsonpath + start + len + 1 ) == '\0'
					   )
					&& ( rc = JJP_ERR_UNSUPPORTED ), final_cleanup );

			end = start + len + 1;

			if( ( ! arr_wildcard ) && index < 0 ) index = tok[parent].size + index;

			for( i = parent; i < tok_count; i++ ) {

				if( tok[i].end > tok[parent].end ) {
					break;
				}

				if( tok[i].parent == parent ) {

					if( ! arr_wildcard ) index--;

					if( arr_wildcard || index == -1 ) {
						parse_recurse( json, tok, tok_count, jsonpath, wrap, i, end + 1 );
					}

				}

			}

		}

	} else {

		end = get_next_point( jsonpath, start );
		check( ( rc = end ) >= 0, final_cleanup );

		if( end == start && *( jsonpath + start ) == '.' ) {
			// .. was written, recurse into all children
			// the jsonpath can't end with ..
			check( *( jsonpath + end + 1 ) != '\0' && ( rc = JJP_ERR_UNSUPPORTED ), final_cleanup );
			recurse = 1;
			start = end + 1;
			end = get_next_point( jsonpath, start );
			check( ( rc = end ) >= 0, final_cleanup );
		} else {
			recurse = 0;
		}

		is_wildcard = ( end - start == 1 && *( jsonpath + start ) == '*' );

		for ( i = parent; i < tok_count; i++ ) {

			if( tok[i].end > tok[parent].end ) {
				break;
			}

			if(
					(
					 tok[i].parent == parent
					 || (
						 recurse
						 && tok[i].start > tok[parent].start
						 && tok[i].end < tok[parent].end
					    )
					)
					&& tok[i].type == JSMN_STRING
					&& (
						is_wildcard
						|| (
							end - start == tok[i].end - tok[i].start
							&& strings_are_equal( json + tok[i].start, jsonpath + start, tok[i].end - tok[i].start )
						   )
					   )

			  ) {
				check( i + 1 < tok_count && tok[i + 1].parent == (int)i && ( rc = JJP_ERR_WEIRD_JSON ), final_cleanup );
				parse_recurse( json, tok, tok_count, jsonpath, wrap, i + 1, end + 1 );
			}
		}

	}


	return;

final_cleanup:
	wrap->error = rc;
#ifndef JJP_NO_MALLOC
	if( wrap->result ) {
		free( wrap->result->tokens );
		wrap->result->count = 0;
	}
#endif
	return;

}




// public functions

#ifndef JJP_NO_MALLOC
jjp_result_t * jjp_jsonpath(
		const char * json,
		jsmntok_t * tokens,
		unsigned int tokens_count,
		const char * jsonpath,
		unsigned int current_object
		) {

	jjp_result_t * result;
	jjp_result_wrapper_t wrap;


	result = malloc( sizeof( jjp_result_t ) );
	check( result, final_cleanup );

	result->count = 0;
	result->tokens = NULL;

	check( jsonpath && ( jsonpath[0] == '$' || jsonpath[0] == '@' ) && jsonpath[1] == '.', result_cleanup );

	if( jsonpath[0] == '$' ) current_object = 0;

	check( current_object < tokens_count && tokens[current_object].type == JSMN_OBJECT, result_cleanup );

	wrap.error = JJP_OK;
	wrap.only_first = -2;
	wrap.result = result;
	wrap.max_mem = 0;
	parse_recurse( json, tokens, tokens_count, jsonpath, &wrap, current_object, 2 );

	check( wrap.error == JJP_OK, result_cleanup );

	return result;

result_cleanup:
	jjp_result_destroy( result );

final_cleanup:
	return NULL;

}

void jjp_result_destroy( jjp_result_t * result ) {
	if( result ) {
		free( result->tokens );
		free( result );
	}
}
#endif

int jjp_jsonpath_first(
		const char * json,
		jsmntok_t * tokens,
		unsigned int tokens_count,
		const char * jsonpath,
		unsigned int current_object
		) {

	jjp_result_wrapper_t wrap;


	check( jsonpath && ( jsonpath[0] == '$' || jsonpath[0] == '@' ) && jsonpath[1] == '.', final_cleanup );

	if( jsonpath[0] == '$' ) current_object = 0;

	check( current_object < tokens_count && tokens[current_object].type == JSMN_OBJECT, final_cleanup );

	wrap.error = JJP_OK;
	wrap.only_first = -1;

	parse_recurse( json, tokens, tokens_count, jsonpath, &wrap, current_object, 2 );
	check( wrap.error == JJP_OK, final_cleanup );

	return wrap.only_first;

final_cleanup:
	return -2;

}

void jjp_jsonpath_save(
		const char * json,
		jsmntok_t * tokens,
		unsigned int tokens_count,
		const char * jsonpath,
		unsigned int current_object,
		int * results,
		unsigned int num_results,
		int * results_count
		) {

	jjp_result_wrapper_t wrap;


	check( jsonpath && ( jsonpath[0] == '$' || jsonpath[0] == '@' ) && jsonpath[1] == '.', final_cleanup );

	if( jsonpath[0] == '$' ) current_object = 0;

	check( current_object < tokens_count && tokens[current_object].type == JSMN_OBJECT, final_cleanup );

	wrap.error = JJP_OK;
	wrap.only_first = -3;
	wrap.count = 0;
	wrap.max_mem = num_results;
	wrap.provided_store = results;

	parse_recurse( json, tokens, tokens_count, jsonpath, &wrap, current_object, 2 );

	if( wrap.error == JJP_ERR_OUT_OF_MEMORY ) {
		*results_count = -3;
		wrap.error = JJP_OK;
	} else {
		*results_count = wrap.count;
	}

	check( wrap.error == JJP_OK, final_cleanup );

	return;

final_cleanup:
	*results_count = -2;
	return;


}


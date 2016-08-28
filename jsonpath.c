#include "jsonpath.h"
#include "dbg.h"
#include <stdlib.h>




// private functions

typedef struct {
	int only_first;
	jjp_result_t * result;
	unsigned int max_mem;
} jjp_result_wrapper_t;

static char strings_are_equal( const char * s1, const char * s2, int len ) {

	int i;

	for( i = 0; i < len; i++ ) {
		if( s1[i] != s2[i] ) return 0;
	}

	return 1;

}

static void add_to_result( jjp_result_wrapper_t * wrap, int token ) {

	unsigned int * tmp;


	if( wrap->only_first == -2 ) {

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

	} else if( wrap->only_first == -1 ) {
		wrap->only_first = token;
	}

	return;

final_cleanup:
	if( wrap->result ) {
		free( wrap->result->tokens );
		wrap->result->tokens = NULL;
		wrap->result->count = 0;
		wrap->result->error = JJP_ERR_OUT_OF_MEMORY;
	}
	return;

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
		if( ( wrap->only_first == -2 && wrap->result->error == JJP_OK ) || wrap->only_first == -1 ) {
			add_to_result( wrap, parent );
		}
	} else if( *( jsonpath + start - 1 ) == '[' ) {
		int index;
		int len;


		if( tok[parent].type == JSMN_ARRAY ) {

			index = to_int( (char *)jsonpath + start, &len );
			check( len > 0 && *( jsonpath + start + len ) == ']'
					&& (
						*( jsonpath + start + len + 1 ) == '.'
						|| *( jsonpath + start + len + 1 ) == '\0'
					   )
					&& ( rc = JJP_ERR_UNSUPPORTED ), final_cleanup );

			end = start + len + 1;

			if( index < 0 ) index = tok[parent].size + index;

			for( i = parent; i < tok_count; i++ ) {

				if( tok[i].end > tok[parent].end ) {
					break;
				}

				if( tok[i].parent == parent ) {

					index--;

					if( index == -1 ) {
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
	if( wrap->result ) {
		wrap->result->error = rc;
		free( wrap->result->tokens );
		wrap->result->count = 0;
	}
	return;

}




// public functions

jjp_err_t jjp_jsonpath(
		const char * json,
		jsmntok_t * tokens,
		unsigned int tokens_count,
		const char * jsonpath,
		unsigned int current_object,
		jjp_result_t * result
		) {

	jjp_result_wrapper_t wrap;


	check( jsonpath && ( jsonpath[0] == '$' || jsonpath[0] == '@' ) && jsonpath[1] == '.', unsupported_cleanup );

	if( jsonpath[0] == '$' ) current_object = 0;

	check( current_object < tokens_count && tokens[current_object].type == JSMN_OBJECT, noobj_cleanup );

	result->error = JJP_OK;
	result->count = 0;
	result->tokens = NULL;

	wrap.only_first = -2;
	wrap.result = result;
	wrap.max_mem = 0;
	parse_recurse( json, tokens, tokens_count, jsonpath, &wrap, current_object, 2 );

	return JJP_OK;

unsupported_cleanup:
	return JJP_ERR_UNSUPPORTED;

noobj_cleanup:
	return JJP_ERR_NOOBJ;

}

jjp_err_t jjp_jsonpath_first(
		const char * json,
		jsmntok_t * tokens,
		unsigned int tokens_count,
		const char * jsonpath,
		unsigned int current_object,
		int * result
		) {

	jjp_result_wrapper_t wrap;


	check( jsonpath && ( jsonpath[0] == '$' || jsonpath[0] == '@' ) && jsonpath[1] == '.', unsupported_cleanup );

	if( jsonpath[0] == '$' ) current_object = 0;

	check( current_object < tokens_count && tokens[current_object].type == JSMN_OBJECT, noobj_cleanup );

	wrap.only_first = -1;
	wrap.result = NULL;
	wrap.max_mem = 0;

	parse_recurse( json, tokens, tokens_count, jsonpath, &wrap, current_object, 2 );

	*result = wrap.only_first;

	return JJP_OK;

unsupported_cleanup:
	return JJP_ERR_UNSUPPORTED;

noobj_cleanup:
	return JJP_ERR_NOOBJ;

}

void jjp_result_deinit( jjp_result_t * result ) {
	free( result->tokens );
}



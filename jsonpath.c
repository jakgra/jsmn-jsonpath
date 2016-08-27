#include "jsonpath.h"
#include "dbg.h"
#include <stdlib.h>
#include <string.h>




// private functions

typedef struct {
	jjp_result_t * result;
	unsigned int max_mem;
} jjp_result_wrapper_t;

static void add_to_result( jjp_result_wrapper_t * wrap, int token ) {

	unsigned int * tmp;


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

	return;

final_cleanup:
	free( wrap->result->tokens );
	wrap->result->tokens = NULL;
	wrap->result->count = 0;
	wrap->result->error = JJP_ERR_OUT_OF_MEMORY;
	return;

}

static inline int get_next_point( const char * jsonpath, int start ) {

	char * cur_c;

	for( cur_c = (char *)jsonpath + start; cur_c - jsonpath < JJP_MAX_JSONPATH_LENGTH; cur_c++ ) {
		if( *cur_c == '.' || *cur_c == '\0' ) {
			break;
		}
	}
	check( cur_c - jsonpath < JJP_MAX_JSONPATH_LENGTH, final_cleanup );

	return cur_c - jsonpath;

final_cleanup:
	return JJP_ERR_TO_LONG;

}

static void parse_recurse( const char * json, jsmntok_t * tok, unsigned int tok_count,
		const char * jsonpath, jjp_result_wrapper_t * wrap, int parent, int start ) {

	jjp_err_t rc;
	int end;
	unsigned int i;
	char is_wildcard;
	char recurse;


	if( *( jsonpath + start - 1 ) == '\0' ) {
		if( wrap->result->error == JJP_OK ) add_to_result( wrap, parent );
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

		for ( i = 1; i < tok_count; i++ ) {
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
							&& strncmp( json + tok[i].start, jsonpath + start, tok[i].end - tok[i].start ) == 0
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
	wrap->result->error = rc;
	free( wrap->result->tokens );
	wrap->result->count = 0;
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

	wrap.result = result;
	wrap.max_mem = 0;
	parse_recurse( json, tokens, tokens_count, jsonpath, &wrap, current_object, 2 );

	return JJP_OK;

unsupported_cleanup:
	return JJP_ERR_UNSUPPORTED;

noobj_cleanup:
	return JJP_ERR_NOOBJ;

}

void jjp_result_deinit( jjp_result_t * result ) {
	free( result->tokens );
}



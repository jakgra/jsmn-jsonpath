#include "jsonpath.h"
#ifndef JJP_NO_STDARG
#include <stdarg.h>
#endif
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

static void parse_recurse( const char * json, jsmntok_t * tok, unsigned int tok_count,
		const char * jsonpath, jjp_result_wrapper_t * wrap, int parent, int start ) {

	jjp_err_t rc;
	char * cur_c;
	int end;
	unsigned int i;



	if( *( jsonpath + start - 1 ) == '\0' ) {
		if( wrap->result->error == JJP_OK ) add_to_result( wrap, parent );
	} else {

		for( cur_c = (char *)jsonpath + start; cur_c - jsonpath < JJP_MAX_JSONPATH_LENGTH; cur_c++ ) {
			if( *cur_c == '.' || *cur_c == '\0' ) {
				end = cur_c - jsonpath;
				break;
			}
		}
		check( start < JJP_MAX_JSONPATH_LENGTH && ( rc = JJP_ERR_TO_LONG ), final_cleanup );

		if( end - start == 0 && *( jsonpath + start ) == '.' ) {
			// .. was written, recurse on next run
			parse_recurse( json, tok, tok_count, jsonpath, wrap, -1, end + 1 );
		} else {

			char is_wildcard;

			is_wildcard = ( end - start == 1 && *( jsonpath + start ) == '*' );

			for ( i = 1; i < tok_count; i++ ) {
				if(
						( parent == -1 || tok[i].parent == parent )
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

	}

	return;

final_cleanup:
	wrap->result->error = rc;
	free( wrap->result->tokens );
	wrap->result->count = 0;
	return;

}
static jjp_err_t parse_jsonpath( const char * json, jsmntok_t * tok, unsigned int tok_count, const char * jsonpath, jjp_result_t * result ) {

	jjp_result_wrapper_t wrap;


	check( tok_count > 0 && tok[0].type == JSMN_OBJECT, noobj_cleanup );

	check( jsonpath && jsonpath[0] == '$' && jsonpath[1] == '.', unsupported_cleanup );

	result->error = JJP_OK;
	result->count = 0;
	result->tokens = NULL;

	wrap.result = result;
	wrap.max_mem = 0;
	parse_recurse( json, tok, tok_count, jsonpath, &wrap, 0, 2 );

	return JJP_OK;

unsupported_cleanup:
	return JJP_ERR_UNSUPPORTED;

noobj_cleanup:
	return JJP_ERR_NOOBJ;

}




// public functions

jjp_err_t jjp_jsonpath(
		const char * json,
		jsmntok_t * tokens,
		unsigned int tokens_count,
		const char * jsonpath,
		jjp_result_t * result
#ifndef JJP_NO_STDARG
		, ...
#endif
		) {

#ifndef JJP_NO_STDARG
	va_list argp;
#endif
	jjp_err_t rc;
	const char * path;


	rc = parse_jsonpath( json, tokens, tokens_count, jsonpath, result );
	check( rc == JJP_OK, final_cleanup );

#ifndef JJP_NO_STDARG
	va_start( argp, result );

	while( ( path = va_arg( argp, const char * ) ) ) {
		rc = parse_jsonpath( json, tokens, tokens_count, path, va_arg( argp, jjp_result_t * ) );
		check( rc == JJP_OK, final_cleanup );
	}
#endif

	return JJP_OK;

final_cleanup:
	return rc;

}

void jjp_result_deinit( jjp_result_t * result ) {
	free( result->tokens );
}



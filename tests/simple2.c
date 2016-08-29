#include "../jsonpath.h"
#include "jsmn/jsmn.h"
#include <stdio.h>
#include <string.h>

const char * json = "{\"pills\": [ \
{ \"color\": \"blue\", \"action\": \"sleep\" }, \
{ \"color\": \"red\", \"action\": \"die\" } \
	] }";

int main() {
	int tok_count;
	jsmntok_t tok[100];
	jsmn_parser parser;

	jsmn_init( &parser );
	tok_count = jsmn_parse( &parser, json, strlen( json ), tok, 100 );

	jjp_result_t * r = jjp_jsonpath( json, tok, tok_count, "$.pills[-1].*", 0 );
	if( r ) {
		for( unsigned int i = 0; i < r->count; i++ ) printf( "Value for key \"%.*s\" is: \"%.*s\"\n",
				tok[ tok[ r->tokens[i] ].parent ].end - tok[ tok[ r->tokens[i] ].parent ].start,
				json + tok[ tok[ r->tokens[i] ].parent ].start,
				tok[ r->tokens[i] ].end - tok[ r->tokens[i] ].start,
				json + tok[ r->tokens[i] ].start );
		jjp_result_destroy( r );
	}

	return 0;
}

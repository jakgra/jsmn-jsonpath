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

	int rs[7];
	int count;
	jjp_jsonpath_save( json, tok, tok_count, "$.pills[-1].*", 0, rs, 7, &count );
	for( int i = 0; i < count; i++ ) printf( "Value for key \"%.*s\" is: \"%.*s\"\n",
			tok[ tok[ rs[i] ].parent ].end - tok[ tok[ rs[i] ].parent ].start,
			json + tok[ tok[ rs[i] ].parent ].start,
			tok[ rs[i] ].end - tok[ rs[i] ].start,
			json + tok[ rs[i] ].start );

	return 0;
}

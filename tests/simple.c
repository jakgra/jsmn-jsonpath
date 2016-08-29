#include "../jsonpath.h"
#include "jsmn/jsmn.h"
#include <stdio.h>
#include <string.h>

const char * json = "{\"pills\": [ { \"color\": \"blue\", \"action\": \"sleep\" }, { \"color\": \"red\", \"action\": \"die\" } ] }";

int main() {
	int tok_count;
	jsmntok_t tok[100];
	jsmn_parser parser;
	int c;

	jsmn_init( &parser );
	tok_count = jsmn_parse( &parser, json, strlen( json ), tok, 100 );

	c = jjp_jsonpath_first( json, tok, tok_count, "$.pills[0].color", 0 );
	printf( "If you take the %.*s pill,",  tok[c].end - tok[c].start, json + tok[c].start );

	c = jjp_jsonpath_first( json, tok, tok_count, "$.pills[0].action", 0 );
	printf( " you may as well just %.*s your life away.\n",  tok[c].end - tok[c].start, json + tok[c].start );

	return 0;
}

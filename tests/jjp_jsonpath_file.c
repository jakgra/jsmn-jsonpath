#include <jsmn.h>
#include <jsonpath.h>
#include <stdio.h>
#include <stdlib.h>


int main( int argc, char * * argv ) {

	int rc;
	size_t rc_s;

	FILE * f;
	char * str;
	size_t str_len;
	jsmn_parser parser;
#define MAX_TOKENS 20000
	jsmntok_t tok[MAX_TOKENS];
	int tok_count;
	jjp_result_t * result;
	unsigned int i;


	check( argc > 2, final_cleanup );

	f = fopen( argv[1], "r" );
	check( f, final_cleanup );
	rc = fseek( f, 0, SEEK_END );
	check( rc == 0, final_cleanup );
	str_len = ftell( f );
	rc = fseek( f, 0, SEEK_SET );
	check( rc == 0, final_cleanup );
	str = malloc( str_len + 1 );
	check( str, final_cleanup );
	rc_s = fread( str, 1, str_len, f );
	check( rc_s == str_len, final_cleanup );
	str[ str_len ] = '\0';
	str_len++;

	jsmn_init( &parser );

	tok_count = jsmn_parse( &parser, str, str_len, tok, MAX_TOKENS );
	check( tok_count > 0, final_cleanup );

	result = jjp_jsonpath( str, tok, tok_count, argv[2], 0 );
	check( result, final_cleanup );

	printf( "Matches count: %d\n", result->count );
	printf( "Matches:\n");

	for( i = 0; i < result->count; i++ ) {

		jsmntok_t cur;
		jsmntok_t key;

		cur = tok[ result->tokens[i] ];
		key = tok[ cur.parent ];
		if( key.type == JSMN_ARRAY ) key = tok[ key.parent ];
		printf( "%u. (token-%u) (key: %.*s): %.*s\n",
				i + 1, result->tokens[i],
				key.end - key.start, str + key.start,
				cur.end - cur.start, str + cur.start );

	}

	jjp_result_destroy( result );
	fclose( f );

	return 0;

final_cleanup:
	return -1;

}

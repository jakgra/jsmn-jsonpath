#include <jsmn.h>
#include <dbg.h>
#include <jsonpath.h>




const char * json = "{ \"store\": { \
		     \"book\": [ \
{ \"category\": \"reference\", \
	\"author\": \"Nigel Rees\", \
		\"title\": \"Sayings of the Century\", \
		\"price\": 8.95 \
}, \
{ \"category\": \"fiction\", \
	\"author\": \"Evelyn Waugh\", \
		\"title\": \"Sword of Honour\", \
		\"price\": 12.99 \
}, \
{ \"category\": \"fiction\", \
	\"author\": \"Herman Melville\", \
		\"title\": \"Moby Dick\", \
		\"isbn\": \"0-553-21311-3\", \
		\"price\": 8.99 \
}, \
{ \"category\": \"fiction\", \
	\"author\": \"J. R. R. Tolkien\", \
		\"title\": \"The Lord of the Rings\", \
		\"isbn\": \"0-395-19395-8\", \
		\"price\": 22.99 \
} \
	], \
	\"bicycle\": { \
	\"color\": \"red\", \
	\"price\": 19.95 \
	} \
	} \
	}";

int main( int argc, char * * argv ) {

	jjp_err_t rc;
	jsmn_parser parser;
#define MAX_TOKENS 100
	jsmntok_t tok[MAX_TOKENS];
	int tok_count;
	jjp_result_t result;
	unsigned int i;

	check( argc == 2, usage_cleanup );

	jsmn_init( &parser );

	tok_count = jsmn_parse( &parser, json, strlen( json ), tok, MAX_TOKENS );
	check( tok_count > 0, final_cleanup );

	rc = jjp_jsonpath( json, tok, tok_count, argv[1],  &result, NULL );
	check( rc == JJP_OK && result.error == JJP_OK, final_cleanup );

	printf( "Matches count: %d\n", result.count );
	printf( "Matches:\n");

	for( i = 0; i < result.count; i++ ) {

		jsmntok_t cur;

		cur = tok[ result.tokens[i] ];
		printf( "%u. : %.*s\n", i + 1, cur.end - cur.start, json + cur.start );

	}

	jjp_result_deinit( &result );

	return 0;

final_cleanup:
	return -1;

usage_cleanup:
	printf( "Usage: ./static-json \"$.store.book[1].author\"\n" );
	return -1;

}

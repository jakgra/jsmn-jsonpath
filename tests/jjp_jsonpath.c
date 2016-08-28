#include <jsmn.h>
#include <dbg.h>
#include <jsonpath.h>
#include <errno.h>
#include <stdlib.h>




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
	unsigned int cur_obj;

	check( argc == 3, usage_cleanup );

	errno = 0;
	cur_obj = strtol( argv[2], NULL, 10 );
	check( errno == 0, usage_cleanup );

	jsmn_init( &parser );

	tok_count = jsmn_parse( &parser, json, strlen( json ), tok, MAX_TOKENS );
	check( tok_count > 0, final_cleanup );

	rc = jjp_jsonpath( json, tok, tok_count, argv[1], cur_obj, &result );
	check( rc == JJP_OK && result.error == JJP_OK, final_cleanup );

	printf( "Matches count: %d\n", result.count );
	printf( "Matches:\n");

	for( i = 0; i < result.count; i++ ) {

		jsmntok_t cur;
		jsmntok_t key;

		cur = tok[ result.tokens[i] ];
		key = tok[ cur.parent ];
		if( key.type == JSMN_ARRAY ) key = tok[ key.parent ];
		printf( "%u. (token-%u) (key: %.*s): %.*s\n",
				i + 1, result.tokens[i],
				key.end - key.start, json + key.start,
				cur.end - cur.start, json + cur.start );

	}

	jjp_result_deinit( &result );

	return 0;

usage_cleanup:
	printf( "Usage:\n./jjp_jsonpath \"$.store.book[1].author\" 0\n./jjp_jsonpath \"@.author\" 5\n" );

final_cleanup:
	return -1;

}

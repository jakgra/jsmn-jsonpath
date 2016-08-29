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

	jsmn_parser parser;
#define MAX_TOKENS 100
	jsmntok_t tok[MAX_TOKENS];
	int tok_count;
#define MAX_RESULTS 100
	int results[MAX_RESULTS];
	int i;
	unsigned int cur_obj;
	int count;

	check( argc == 3, usage_cleanup );

	errno = 0;
	cur_obj = strtol( argv[2], NULL, 10 );
	check( errno == 0, usage_cleanup );

	jsmn_init( &parser );

	tok_count = jsmn_parse( &parser, json, strlen( json ), tok, MAX_TOKENS );
	check( tok_count > 0, final_cleanup );

	jjp_jsonpath_save( json, tok, tok_count, argv[1], cur_obj, results, MAX_RESULTS, &count );
	check( count != -2, final_cleanup );

	if( count == -3 ) {
		printf( "Provided results array was to small, some results won't be visible !!!\n" );
		count = MAX_RESULTS;
	}
	printf( "Matches count: %d\n", count );
	printf( "Matches:\n");

	for( i = 0; i < count; i++ ) {

		jsmntok_t cur;
		jsmntok_t key;

		cur = tok[ results[i] ];
		key = tok[ cur.parent ];
		if( key.type == JSMN_ARRAY ) key = tok[ key.parent ];
		printf( "%u. (token-%u) (key: %.*s): %.*s\n",
				i + 1, results[i],
				key.end - key.start, json + key.start,
				cur.end - cur.start, json + cur.start );

	}

	return 0;

usage_cleanup:
	printf( "Usage:\n./jjp_jsonpath_save \"$.store.book[1].author\" 0\n./jjp_jsonpath_save \"@.author\" 5\n" );

final_cleanup:
	return -1;

}

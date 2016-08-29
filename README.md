# Jsmn JSONpath
A JSONPath (http://goessner.net/articles/JsonPath/) implementation for jsmn (https://github.com/zserge/jsmn).  
The library should be very portable as there are no dependencies if compiled with `JJP_NO_MALLOC`, not even libc
and otherwise the only dependency is `malloc()` and `free()` from `stdlib.h`.  

## Usage
It is very simple:
```c
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
        int c;

        jsmn_init( &parser );
        tok_count = jsmn_parse( &parser, json, strlen( json ), tok, 100 );

        c = jjp_jsonpath_first( json, tok, tok_count, "$.pills[0].color", 0 );
        printf( "If you take the %.*s pill,",  tok[c].end - tok[c].start, json + tok[c].start );

        c = jjp_jsonpath_first( json, tok, tok_count, "$.pills[0].action", 0 );
        printf( " you may as well just %.*s your life away.\n",  tok[c].end - tok[c].start, json + tok[c].start );

        return 0;
}
```
If you want more than one result then just use this function:
```c
...
jjp_result_t * r = jjp_jsonpath( json, tok, tok_count, "$.pills[-1].*", 0 );
if( r ) {
	for( unsigned int i = 0; i < r->count; i++ ) printf( "Value for key \"%.*s\" is: \"%.*s\"\n",
			tok[ tok[ r->tokens[i] ].parent ].end - tok[ tok[ r->tokens[i] ].parent ].start,
			json + tok[ tok[ r->tokens[i] ].parent ].start,
			tok[ r->tokens[i] ].end - tok[ r->tokens[i] ].start,
			json + tok[ r->tokens[i] ].start );
	jjp_result_destroy( r );
}
```
If you hate dynamic memory allocation :) use this instead:
```c
...
int rs[7];
int count;
jjp_jsonpath_save( json, tok, tok_count, "$.pills[-1].*", 0, rs, 7, &count );
for( int i = 0; i < count; i++ ) printf( "Value for key \"%.*s\" is: \"%.*s\"\n",
		tok[ tok[ rs[i] ].parent ].end - tok[ tok[ rs[i] ].parent ].start,
		json + tok[ tok[ rs[i] ].parent ].start,
		tok[ rs[i] ].end - tok[ rs[i] ].start,
		json + tok[ rs[i] ].start );
```
This four ( `jjp_jsonpath_first()`, `jjp_jsonpath()`, `jjp_result_destroy()` and `jjp_jsonpath_save()` ) are all the functions the library has.  
You should also note that to increase performance you should save results from similar JSONPaths and use them as current elements.  
You can do this in the first example ( tests/simple.c ) if you rewrite it as:
```c
...
int p = jjp_jsonpath_first( json, tok, tok_count, "$.pills[0]", 0 );

c = jjp_jsonpath_first( json, tok, tok_count, "@.color", p );
printf( "If you take the %.*s pill,",  tok[c].end - tok[c].start, json + tok[c].start );

c = jjp_jsonpath_first( json, tok, tok_count, "@.action", p );
printf( " you may as well just %.*s your life away.\n",  tok[c].end - tok[c].start, json + tok[c].start );
...
```
Also have a look at jsonpath.h as it is heavily commented.
For simple full examples look in the tests directory. For the implementation to work jsmn must be compiled with parent links.
You can normally (if you use gcc or clang) achieve this by adding the `-DJSMN_PARENT_LINKS` option to the compile command.

## The JSONPath implementation
Here are some examples how the implementation parses some JSONPaths for the following json:
```json
{ "store": {
	"book": [
	{ "category": "reference",
		"author": "Nigel Rees",
		"title": "Sayings of the Century",
		"price": 8.95
	},
	{ "category": "fiction",
		"author": "Evelyn Waugh",
		"title": "Sword of Honour",
		"price": 12.99
	},
	{ "category": "fiction",
		"author": "Herman Melville",
		"title": "Moby Dick",
		"isbn": "0-553-21311-3",
		"price": 8.99
	},
	{ "category": "fiction",
		"author": "J. R. R. Tolkien",
		"title": "The Lord of the Rings",
		"isbn": "0-395-19395-8",
		"price": 22.99
	}
	],
	"bicycle": {
		"color": "red",
		"price": 19.95
	}
	   }
}
```
  
You can try them like this:
```bash
cd tests
./build.sh
./jjp_parse "$JSONPATH" "$CURRENT_ELEMENT"
```
  
When the jsonpath begins with `$.` the current element is ignored and can be anything beetween `0` and `tokens_count - 1`  
  
|         JSONPATH | CURRENT\_ELEMENT |              RESULT DESCRIPTION |
| ---------------- | ---------------- | ------------------------------- |
|       $.store.\* |                0 |       book and bicycle elements |
| $.\*.book[-3].\* |                0 | all elements in the second book |
|         @..price |                0 |              all price elements |
|         @..price |               46 |        the price of the bicycle |
|             @.\* |               46 |  color and price of the bicycle |
|        $..author |                0 |             all author elements |
  
## Compile options
You can control the behaviour of jsmn-jsonpath with a few compile options:  
1. `JJP_NO_MALLOC` if defined disables the use of malloc and free so there are no dependencies and no memory allocations at all ( no `stdlib.h` ... ). Note that the `jjp_jsonpath()` function becomes unavailable in this case and you have to use the `jjp_jsonpath_save()` function instead.  
2. `JJP_LOG` if defined enables logging of all jjp errors to stderr (look at dbg.h for more info). Primarly useful for developing
and debugging jsmn-jsonpath.  
  
## TODO:  
- add automated tests ( also valgrind test for leaks )
- add support for wildcard in array operator ( `book[*]` )
- add support for filter expressions ( `$.store.book[?(@.price < 10)].title` )
- maybe add other operators( union, array slice, ... )  

## Bugs
I am sure there are many...  
Please report them on github as new issues.

## Contributing
All contributions, questions, interest, usage, ... is warmly welcomed ... :)  

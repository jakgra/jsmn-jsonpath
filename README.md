# Jsmn JSONpath
A JSONPath (http://goessner.net/articles/JsonPath/) implementation for jsmn (https://github.com/zserge/jsmn).  
The library should be very portable as the only dependency is `malloc()` and `free()` from `stdlib.h`.  

## Usage
For an example look in the tests directory. For the implementation to work jsmn must be compiled with parent links.
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
1. `JJP_NO_MALLOC` if defined disables the use of malloc and free so there are no dependencies at all ( no stdlib.h ... ). Note that the api changes slightly.  
2. `JJP_LOG` if defined enables logging of all jjp errors to stderr (look at dbg.h for more info). Primarly useful for developing
and debugging jsmn-jsonpath.  
  
## TODO:  
- add automated tests ( also valgrind test for leaks )
- add support for wildcard in array operator ( `book[*]` )
- add support for `JJP_NO_MALLOC`
- add support for filter expressions ( `$.store.book[?(@.price < 10)].title` )
- maybe add other operators( union, array slice, ... )  

## Bugs
I am sure there are many...  
Please report them on github as new issues.

## Contributing
All contributions, questions, interest, usage, ... is warmly welcomed ... :)  

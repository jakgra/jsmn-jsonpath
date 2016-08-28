# Jsmn JSONpath
A JSONPath (http://goessner.net/articles/JsonPath/) implementation for jsmn (https://github.com/zserge/jsmn).  

## THIS IS STILL IN BETA

## Usage
For an example look in the tests directory. For the implementation to work jsmn must be compiled with parent links.
You can normally (if you use gcc or clang) achieve this by adding the `-DJSMN_PARENT_LINKS` option to the compile command.

## Compile options
You can control the behaviour of jsmn-jsonpath with a few compile options:  
1. `JJP_NO_MALLOC` if defined disables the use of malloc and free so there are no dependencies at all ( no stdlib.h ... ). Note that the api changes slightly.  
2. `JJP_LOG` if defined enables logging of all jjp errors to stderr (look at dbg.h for more info). Primarly useful for developing
and debugging jsmn-jsonpath.  
  
## TODO:  
- add support for wildcard in array operator ( `book[*]` )
- add support for `JJP_NO_MALLOC`
- add support for filter expressions ( `$.store.book[?(@.price < 10)].title` )
- maybe add other operators( union, array slice, ... )  


## Bugs
I am sure there are many...  
Please report them on github as new issues.

#ifndef jjp_jjp_wrap_h
#define jjp_jjp_wrap_h

#include <jsonpath.h>




float jjp_float( const char * json, jsmntok_t * tokens, unsigned int tokens_count, const char * jsonpath, unsigned int current_object, int * success );

double jjp_double( const char * json, jsmntok_t * tokens, unsigned int tokens_count, const char * jsonpath, unsigned int current_object, int * success );

int jjp_int( const char * json, jsmntok_t * tokens, unsigned int tokens_count, const char * jsonpath, unsigned int current_object, int * success );

long jjp_long( const char * json, jsmntok_t * tokens, unsigned int tokens_count, const char * jsonpath, unsigned int current_object, int * success );

int jjp_boolean( const char * json, jsmntok_t * tokens, unsigned int tokens_count, const char * jsonpath, unsigned int current_object, int * success );




#endif

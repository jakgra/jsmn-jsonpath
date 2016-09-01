#include <jjp_wrap.h>
#include <jsonpath.h>
#include <stdlib.h>
#include <errno.h>




float jjp_float( const char * json, jsmntok_t * tokens, unsigned int tokens_count, const char * jsonpath, unsigned int current_object, int * success ) {

        float r;
        int i;


        i = jjp_jsonpath_first( json, tokens, tokens_count, jsonpath, current_object );
        check( i >= 0 && tokens[i].type == JSMN_PRIMITIVE, final_cleanup );

        errno = 0;
        r = strtof( json + tokens[i].start, NULL );
        check( errno == 0, final_cleanup );

        if( success ) *success = 0;

        return r;

final_cleanup:
        if( success ) *success = -1;
        return -1;

}

int jjp_int( const char * json, jsmntok_t * tokens, unsigned int tokens_count, const char * jsonpath, unsigned int current_object, int * success ) {

        int r;
        int i;


        i = jjp_jsonpath_first( json, tokens, tokens_count, jsonpath, current_object );
        check( i >= 0 && tokens[i].type == JSMN_PRIMITIVE, final_cleanup );

        errno = 0;
        r = strtol( json + tokens[i].start, NULL, 10 );
        check( errno == 0, final_cleanup );

        if( success ) *success = 0;

        return r;

final_cleanup:
        if( success ) *success = -1;
        return -1;

}

long jjp_long( const char * json, jsmntok_t * tokens, unsigned int tokens_count, const char * jsonpath, unsigned int current_object, int * success ) {

        long r;
        int i;


        i = jjp_jsonpath_first( json, tokens, tokens_count, jsonpath, current_object );
        check( i >= 0 && tokens[i].type == JSMN_PRIMITIVE, final_cleanup );

        errno = 0;
        r = strtoll( json + tokens[i].start, NULL, 10 );
        check( errno == 0, final_cleanup );

        if( success ) *success = 0;

        return r;

final_cleanup:
        if( success ) *success = -1;
        return -1;

}


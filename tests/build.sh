#!/bin/sh

cc -g -Wall -Wextra -pedantic -std=c99 -DJSMN_PARENT_LINKS -DJJP_LOG \
	-Ijsmn -I.. jsmn/jsmn.c ../jsonpath.c jjp_jsonpath.c -o jjp_jsonpath
cc -g -Wall -Wextra -pedantic -std=c99 -DJSMN_PARENT_LINKS -DJJP_NO_MALLOC -DJJP_LOG \
	-Ijsmn -I.. jsmn/jsmn.c ../jsonpath.c jjp_jsonpath_first.c -o jjp_jsonpath_first
cc -g -Wall -Wextra -pedantic -std=c99 -DJSMN_PARENT_LINKS -DJJP_NO_MALLOC -DJJP_LOG -Ijsmn \
	-I.. jsmn/jsmn.c ../jsonpath.c jjp_jsonpath_save.c -o jjp_jsonpath_save


#!/bin/sh

cc -g -Wall -Wextra -pedantic -std=gnu99 -DJSMN_PARENT_LINKS -DJJP_LOG -Ijsmn -I.. jsmn/jsmn.c ../jsonpath.c static-json.c -o static-json


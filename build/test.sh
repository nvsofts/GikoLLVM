#!/bin/sh

./giko < sum.gikob && clang -O2 -o test out.bc stdlib.c && ./test

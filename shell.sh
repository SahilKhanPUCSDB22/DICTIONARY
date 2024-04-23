#!/usr/bin/bash

wc -c $2 | grep "[0-9]*" -o >inpfile
gcc motrie.c -lm -g
./a.out < inpfile  $1 $2

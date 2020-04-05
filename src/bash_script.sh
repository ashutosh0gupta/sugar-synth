#!/bin/sh

cd ./build/depqbf/examples
gcc -o depqbf-file depqbf-file.c -L.. -lqdpll
./depqbf-file

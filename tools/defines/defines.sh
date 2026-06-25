#!/bin/sh

gcc -Wall -Wextra limits.c
./a.out
gcc -Wall -Wextra float.c
./a.out
gcc -Wall -Wextra stdint.c
./a.out

clang -Weverything limits.c
./a.out
clang -Weverything float.c
./a.out
clang -Weverything stdint.c
./a.out
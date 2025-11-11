#!/bin/sh

set -e

# The name of the source sans prefix.
x=$(basename -- $1)

# The name of the source sans extension.
y=${x%.*}

mkdir -p .build
as -o .build/$y.o $1
ld -o $y .build/$y.o

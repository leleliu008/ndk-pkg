#!/bin/sh

set -ex

for f in *.c
do
    o="${f%.c}"
    cc -flto -Os -std=c99 -o "$o" "$f"
    strip "$o"
    mv "$o" ~/.ndk-pkg/core/
done

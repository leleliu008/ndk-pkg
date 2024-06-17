#!/bin/sh

set -ex

for item in *.c
do
    file="${item%.c}"
    cc -flto -Os -std=c99 -Wl,-s -o "$file" "$item"
    mv "$file" ~/.ndk-pkg/core/
done

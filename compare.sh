#!/bin/bash

file1="send.txt"
file2="receive.txt"

if cmp -s "$file1" "$file2"; 
then
    printf '"%s" is the same as "%s"\n' "$file1" "$file2"
else
    printf '"%s" is different from "%s"\n' "$file1" "$file2"
fi
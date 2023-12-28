#!/bin/bash

if test $# -ne 1; then
    echo "Usage: ./script.sh <c>"
    exit 1
fi

char="$1"
count=0

while IFS= read -r line; do  
    if echo "$line" | 
        grep -E "^[A-Z][A-Za-z0-9 ,.!?]*" | 
        grep -E "[.!?]$" | 
        grep -Ev ",[ ]*si" |
        grep -q "$char"
    then
        ((count++))
    fi
done

echo -e "$count"
exit 1
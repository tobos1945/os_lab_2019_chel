#!/bin/bash
a=$#
sum=0
echo "всего $a"
for i in $@; do sum=$((sum+i)); done
echo "$((sum / a))"

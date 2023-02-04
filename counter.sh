#!/bin/bash

line_count=0
for file in $(find . -name "*.hpp" -o -name "*.cc")
do
  lines=`wc -l $file | awk '{print $1}'`
  line_count=$((line_count + lines))
done

echo "C++ code line count: $line_count"


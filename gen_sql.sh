#!/bin/bash
for id in {1..100}
do
  echo "insert into stu values (${id}, 10, 'student${id}' );" >> test.sql
done
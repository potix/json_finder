#!/bin/sh
# function loading
#echo "CREATE FUNCTION jfmin RETURNS string SONAME 'mysql_json_finder.so.0.0.0';"
#echo "CREATE FUNCTION jfget RETURNS string SONAME 'mysql_json_finder.so.0.0.0';"
#echo "CREATE FUNCTION jfgetint RETURNS integer SONAME 'mysql_json_finder.so.0.0.0';"
#echo "CREATE FUNCTION jfgetreal RETURNS real SONAME 'mysql_json_finder.so.0.0.0';"
#
echo "drop database json_finder_test;"
echo "create database json_finder_test;"
echo "use json_finder_test;"
echo "START TRANSACTION;"
echo "create table data (k integer, v text);"
for i in $(seq 20000)
do
    json="{\"name\": \"user${i}\", \"id\":\"${i}\", \"person\": { \"age\": ${i}, \"group\": \"user\", \"uuid\":\"$(uuidgen)\"} }" 
    echo "insert into data values($(date +%s), jfmin('${json}'));"
done
echo "create index data_k on data (k);"
echo "COMMIT;"
echo "select k,jfget(v, 'name'),jfget(v, 'id'),jfget(v, 'person.age'),jfget(v, 'person.uuid'),jfget(v, 'dummy') from data where jfget(v, 'person.uuid') like '1%' and jfgetint(v, 'person.age') < 10000 and jfget(v, 'dummy') is null limit 10;"
echo "select * from data limit 10;"

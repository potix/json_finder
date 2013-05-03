#!/bin/sh
echo "pragma encoding=utf8;"
echo "select load_extension('../sqlite_json_finder.so.0.0.0');"
echo "BEGIN TRANSACTION;"
echo "create table data (key integer, value text);"
for i in $(seq 20000)
do
    json="{\"name\": \"user${i}\", \"id\":\"${i}\", \"person\": { \"age\": ${i}, \"group\": \"user\", \"uuid\":\"$(uuidgen)\"} }" 
    echo "insert into data values ($(date +%s), jfmin('${json}'));"
done
echo "create index data_key on data (key);"
echo "COMMIT;"
echo "select key,jfget(value, 'name'),jfget(value, 'id'),jfget(value, 'person.age'),jfget(value, 'person.uuid'),jfget(value, 'dummy') from data where jfget(value, 'person.uuid') like '1%' and jfget(value, 'person.age') < 10000 and jfget(value, 'dummy') is null limit 10;"
echo "select * from data limit 10;"

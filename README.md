# json finder

## dependency
* sqlite3 (need sqlite3ext.h)

## about json finder
* json finder is library for find value from json data.
* json finder is udf of sqlite.
* json finder is created on the basis of the vjson code.

## license
* MIT license

## compile
### library 
    > make

### test code
    > make test
    > ./json_finder_test
    > make prof
    > ./json_finder_prof

## example 
    > cd example
    > ./create_test_sql.sh | sqlite3 test.db

## udf functions
* jfmin(json_text)
  * minimize function.
  * one argument only.
  * argument is json text.

* jfget(json_column_name, 'finding_key')
  * find function.
  * two arguments.
  * first argument is coluanm name of json text.
  * second argument is key for finding.
  * nested key is separate by '.'.
    * e.g: { "a" : { "b" : 1 } } -> 'a.b'

## restrictions
* not supported nesting 32 times or more.
  * but this limit change by -DMAX_NEST define with compile option.
* not supported key length exceeds 1024byte.
  * but this limit change by -DMAX_PATH_LEN define with compile option.
* not supported UTF-16.

## TODO
* speed up.
* reduce restrictions
* sufficient test
* mysql support.



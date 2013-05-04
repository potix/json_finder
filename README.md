# json finder

## dependency
* sqlite3 (need sqlite3ext.h)

## about json finder
* json finder is library for find value from json string.
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

### sqlite
* jfmin(json_text)
    * minimization function.
    * one argument only.
    * argument is json text.

* jfget(json_column_name, 'finding_key')
    * finding function.
    * two arguments.
    * first argument is coluanm name of json string.
    * second argument is key for finding.
    * nested key is separate by '.'.
        * e.g: { "a" : { "b" : 1 } } -> 'a.b', { "a" : [ "b" ] }  -> 'a.0'
    * if key include '.', escape by '\\'
        * e.g:  { "a.txt" : { "content" : "hello!!" } } -> 'a\\.txt.content'

### mysql
* jfmin(json_text)
    * minimization function.
    * one argument only.
    * argument is json text.

* jfget(json_column_name, 'finding_key')
    * finding function.
    * two arguments.
    * first argument is coluanm name of json string.
    * second argument is key for finding.
    * return string or null
        * integer, real and boolean are returned as string
    * nested key is separate by '.'.
        * e.g: { "a" : { "b" : 1 } } -> 'a.b', { "a" : [ "b" ] }  -> 'a.0'
    * if key include '.', escape by '\\'
        * e.g:  { "a.txt" : { "content" : "hello!!" } } -> 'a\\.txt.content'

* jfget_int(json_column_name, 'finding_key')
    * finding function.
    * two arguments.
    * first argument is coluanm name of json string.
    * second argument is key for finding.
    * return integer or null
        * boolean is returned as integer
    * nested key is separate by '.'.
    * if key include '.', escape by '\\'

* jfget_real(json_column_name, 'finding_key')
    * finding function.
    * two arguments.
    * first argument is coluanm name of json string.
    * second argument is key for finding.
    * return real or null
    * nested key is separate by '.'.
    * if key include '.', escape by '\\'

## restrictions
* not supported UTF-16.

## TODO
* sufficient test.

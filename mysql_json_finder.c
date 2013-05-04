#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <mysql/mysql.h>

#include "json_finder.h"

my_bool
jfget_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
	if (args->arg_count != 2) {
		strcpy(message, "jfget() requires two arguments");
		return 1;
	}
	if (args->arg_type[0] != STRING_RESULT) {
		strcpy(message, "jfget() requires a string for first argument");
		return 1;
	}
	if (args->arg_type[1] != STRING_RESULT) {
		strcpy(message, "jfget() requires a string for second argument");
		return 1;
	}
	args->maybe_null[0] = 0;
	args->maybe_null[1] = 0;
	initid->ptr = NULL;
	return 0;
}

void
jfget_deinit(UDF_INIT *initid)
{
	free(initid->ptr);
	return;
}
 
char *
jfget(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
	
        const char *desc;
        json_elem_t elem;
        char *uestr;
        size_t uestr_size;

        switch (json_finder_find(
            &elem,
            (const char *)args->args[0],
            (ssize_t)args->lengths[0],
            (char *)args->args[1],
            (ssize_t)args->lengths[1],
            NULL,
            &desc,
            NULL)) {
        case 1:
		strcpy(result, "parse error");
		*length = sizeof("parse error") - 1;
		*error = 1;
		return NULL;
        case -1:
		*length = 0;
		*is_null = 1;
		return NULL;
        default:
                break;
        }
        switch (elem.type) {
        case JSON_BOOL:
                if (elem.value.b) {
			*length = sizeof("true") - 1;
			return "true";
                } else {
			*length = sizeof("false") - 1;
			return "false";
                }
        case JSON_INTEGER:
                //sqlite3_result_int64(context, elem.value.ll);
                return NULL;
        case JSON_DOUBLE:
                //sqlite3_result_double(context, elem.value.d);
                return NULL;
        case JSON_STRING:
                if (json_finder_unescape_strdup(&uestr, &uestr_size, &elem.value.s)) {
			strcpy(result, "could not unescape and copy");
			*length = sizeof("could not unescape and copy") - 1;
			*error = 1;
			return NULL;
                }
		*length = uestr_size;
		initid->ptr = uestr;
		return uestr;
        case JSON_NULL:
		*length = 0;
		*is_null = 1;
		return NULL;
        default:
		strcpy(result, "unexpedted type");
		*length = sizeof("unexpedted type") - 1;
		*error = 1;
		return NULL;
        }
}

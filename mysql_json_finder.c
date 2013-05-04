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
	    NULL,
	    NULL)) {
	case 1:
		*error = 1;
		return NULL;
	case -1:
		*is_null = 1;
		return NULL;
	default:
		break;
	}
	switch (elem.type) {
	case JSON_BOOL:
		if (elem.value.b) {
			strcpy(result, "true");
			*length = sizeof("true") - 1;
			return result;
		} else {
			strcpy(result, "false");
			*length = sizeof("false") - 1;
			return result;
		}
	case JSON_INTEGER:
		memcpy(result, elem.value.ll.s.ptr, elem.value.ll.s.len);
		result[elem.value.ll.s.len] = '\0';
		*length = elem.value.ll.s.len;
		return result;
	case JSON_DOUBLE:
		memcpy(result, elem.value.d.s.ptr, elem.value.d.s.len);
		result[elem.value.d.s.len] = '\0';
		*length = elem.value.d.s.len;
		return result;
	case JSON_STRING:
		if (json_finder_unescape_strdup(&uestr, &uestr_size, &elem.value.s)) {
			*error = 1;
			return NULL;
		}
		*length = uestr_size - 1;
		initid->ptr = uestr;
		return uestr;
	case JSON_NULL:
		*is_null = 1;
		return NULL;
	default:
		*error = 1;
		return NULL;
	}
}

my_bool
jfgetint_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
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
	return 0;
}

void
jfgetint_deinit(UDF_INIT *initid)
{
	return;
}

long long
jfgetint(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
	json_elem_t elem;

	switch (json_finder_find(
	    &elem,
	    (const char *)args->args[0],
	    (ssize_t)args->lengths[0],
	    (char *)args->args[1],
	    (ssize_t)args->lengths[1],
	    NULL,
	    NULL,
	    NULL)) {
	case 1:
		*error = 1;
		return 0;
	case -1:
		*is_null = 1;
		return 0;
	default:
		break;
	}
	switch (elem.type) {
	case JSON_BOOL:
		return (long long)elem.value.b;
	case JSON_INTEGER:
		return elem.value.ll.v;
	case JSON_DOUBLE:
		*error = 1;
		return 0;
	case JSON_STRING:
		*error = 1;
		return 0;
	case JSON_NULL:
		*is_null = 1;
		return 0;
	default:
		*error = 1;
		return 0;
	}
}

my_bool
jfgetreal_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
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
	return 0;
}

void
jfgetreal_deinit(UDF_INIT *initid)
{
	return;
}

double
jfgetreal(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
	json_elem_t elem;

	switch (json_finder_find(
	    &elem,
	    (const char *)args->args[0],
	    (ssize_t)args->lengths[0],
	    (char *)args->args[1],
	    (ssize_t)args->lengths[1],
	    NULL,
	    NULL,
	    NULL)) {
	case 1:
		*error = 1;
		return 0;
	case -1:
		*is_null = 1;
		return 0;
	default:
		break;
	}
	switch (elem.type) {
	case JSON_BOOL:
		*error = 1;
		return 0;
	case JSON_INTEGER:
		*error = 1;
		return 0;
	case JSON_DOUBLE:
		return elem.value.d.v;
	case JSON_STRING:
		*error = 1;
		return 0;
	case JSON_NULL:
		*is_null = 1;
		return 0;
	default:
		*error = 1;
		return 0;
	}
}

my_bool
jfmin_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
	if (args->arg_count != 1) {
		strcpy(message, "jfget() requires one arguments");
		return 1;
	}
	if (args->arg_type[0] != STRING_RESULT) {
		strcpy(message, "jfget() requires a string");
		return 1;
	}
	args->maybe_null[0] = 0;
	initid->ptr = NULL;
	return 0;
}

void
jfmin_deinit(UDF_INIT *initid)
{
	free(initid->ptr);
	return;
}
 
char *
jfmin(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
        char *json_min;
        ssize_t json_min_size;

        if (json_finder_minimize(
            &json_min,
            &json_min_size,
	    (const char *)args->args[0],
            (ssize_t)args->lengths[0])) {
		*error = 1;
                return NULL;
        }
	initid->ptr = json_min;
	*length = json_min_size - 1;
	return json_min;
}

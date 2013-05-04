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
	char *str;

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
		*length = 0;
		*is_null = 1;
		return NULL;
	default:
		break;
	}
	switch (elem.type) {
	case JSON_BOOL:
		if (elem.value.b) {
			str = malloc(sizeof("true"));
			if (str == NULL) {
				*error = 1;
				return NULL;
			}
			strcpy(str, "true");
			*length = sizeof("true") - 1;
			initid->ptr = str;
			return str;
		} else {
			str = malloc(sizeof("false"));
			if (str == NULL) {
				*error = 1;
				return NULL;
			}
			strcpy(str, "false");
			*length = sizeof("false") - 1;
			initid->ptr = str;
			return str;
		}
	case JSON_INTEGER:
		str = malloc(elem.value.ll.s.len + 1);
		memcpy(str, elem.value.ll.s.ptr, elem.value.ll.s.len);
		str[elem.value.ll.s.len] = '\0';
		*length = elem.value.ll.s.len;
		initid->ptr = str;
		return str;
	case JSON_DOUBLE:
		str = malloc(elem.value.d.s.len + 1);
		memcpy(str, elem.value.d.s.ptr, elem.value.d.s.len);
		str[elem.value.d.s.len] = '\0';
		*length = elem.value.d.s.len;
		initid->ptr = str;
		return str;
	case JSON_STRING:
		if (json_finder_unescape_strdup(&uestr, &uestr_size, &elem.value.s)) {
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
		*error = 1;
		return NULL;
	}
}

my_bool
jfget_integer_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
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
jfget_integer_deinit(UDF_INIT *initid)
{
	return;
}

long long
jfget_integer(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
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
jfget_real_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
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
jfget_real_deinit(UDF_INIT *initid)
{
	return;
}

double jfget_real(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
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

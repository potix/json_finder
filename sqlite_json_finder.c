#include <stdlib.h>
#include <stdint.h>
#include <sqlite3ext.h>

#include "json_finder.h"

SQLITE_EXTENSION_INIT1

static void
json_find(
    sqlite3_context *context,
    int argc,
    sqlite3_value **argv)
{
	const char *desc;
	json_elem_t elem;
	char *uestr;
	size_t uestr_size;

	if (sqlite3_value_type(argv[0]) != SQLITE_TEXT) {
		sqlite3_result_error(context, "Invalid argument (0).", -1);
		return;
	}
	if (sqlite3_value_type(argv[1]) != SQLITE_TEXT) {
		sqlite3_result_error(context, "Invalid argument (1).", -1);
		return;
	}
	switch (json_finder_find(
	    &elem,
	    (const char *)sqlite3_value_text(argv[0]),
	    (ssize_t)sqlite3_value_bytes(argv[0]),
	    (char *)sqlite3_value_text(argv[1]),
	    (ssize_t)sqlite3_value_bytes(argv[1]),
            NULL,
            &desc,
            NULL)) {
	case 1:
		sqlite3_result_error(context, desc, -1);
		return;
	case -1:
		sqlite3_result_null(context);
		return;
	default:
		break;
	}
	switch (elem.type) {
	case JSON_BOOL:
		if (elem.value.b) {
			sqlite3_result_text(context, "true", -1, NULL);
		} else {
			sqlite3_result_text(context, "false", -1, NULL);
		}
		break;
	case JSON_INTEGER:
		sqlite3_result_int64(context, elem.value.ll.v);
		break;
	case JSON_DOUBLE:
		sqlite3_result_double(context, elem.value.d.v);
		break;
	case JSON_STRING:
		if (json_finder_unescape_strdup(&uestr, &uestr_size, &elem.value.s)) {
			sqlite3_result_error(context, "Failed to allocate memory.", -1);
			return;
		}
		sqlite3_result_text(context, uestr, uestr_size, json_finder_free);
		break;
	case JSON_NULL:
		sqlite3_result_null(context);
		break;;
	default:
		sqlite3_result_error(context, "Unexpected value type.", -1);
		break;
	}
}

static void
json_minimize(
    sqlite3_context *context,
    int argc,
    sqlite3_value **argv)
{
	char *json_min;
	ssize_t json_min_size;

	switch (sqlite3_value_type(argv[0])) {
	case SQLITE_TEXT:
		break;
	default:
		sqlite3_result_error(context, "Invalid argument (1).", -1);
		return;
	}
	if (json_finder_minimize(
	    &json_min,
	    &json_min_size,
	    (const char *)sqlite3_value_text(argv[0]),
	    (ssize_t)sqlite3_value_bytes(argv[0]))) {
		sqlite3_result_error(context, "Minimize error", -1);
		return;
	}
	sqlite3_result_text(
	    context,
	    json_min,
	    json_min_size,
	    json_finder_free);
}

int
sqlite3_extension_init(
    sqlite3 *db,
    char **errmsg,
    const sqlite3_api_routines *api)
{
	SQLITE_EXTENSION_INIT2(api)

#ifdef sqlite3_create_function_v2
	if (sqlite3_create_function_v2(db, "jfget", 2, SQLITE_UTF8, NULL, json_find, NULL, NULL, NULL) != SQLITE_OK) {
#else
	if (sqlite3_create_function(db, "jfget", 2, SQLITE_UTF8, NULL, json_find, NULL, NULL) != SQLITE_OK) {
#endif
		*errmsg = sqlite3_mprintf("Failed to create function 'v'.");
		return 1;
	}

#ifdef sqlite3_create_function_v2
	if (sqlite3_create_function_v2(db, "jfmin", 1, SQLITE_UTF8, NULL, json_minimize, NULL, NULL, NULL) != SQLITE_OK) {
#else
	if (sqlite3_create_function(db, "jfmin", 1, SQLITE_UTF8, NULL, json_minimize, NULL, NULL) != SQLITE_OK) {
#endif
		*errmsg = sqlite3_mprintf("Failed to create function 'm'.");
		return 1;
	}

	return 0;
}

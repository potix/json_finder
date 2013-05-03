#ifndef JSON_FINDER_H
#define JSON_FINDER_H

#define json_finder_free free

typedef struct json_string json_string_t;
typedef union json_value json_value_t;
typedef struct json_elem json_elem_t;
typedef struct json_finder json_finder_t;

#define JSON_NULL    1
#define JSON_STRING  2
#define JSON_INTEGER 3
#define JSON_DOUBLE  4
#define JSON_BOOL    5
#define JSON_ARRAY   6
#define JSON_OBJECT  7

struct json_string {
	const char *ptr;
	uint32_t len;
};

union json_value {
	int b;
	json_string_t s;
	long long ll;
	double d;
};

struct json_elem {
	json_value_t value;
        json_string_t name;
	uint32_t elem_idx;
        uint8_t type;
};

int json_finder_find(
    json_elem_t *target,
    const char *json,
    ssize_t json_size,
    const char *key,
    char nest_separator,
    char const **error_positon,
    char const **error_description,
    int *error_line);

char *
json_finder_unescape_strdup(
    json_string_t *string);

int
json_finder_minimize(
    char **json_min,
    size_t *json_min_size,
    const char *json);

#endif

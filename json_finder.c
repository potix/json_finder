#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "json_finder.h"

#ifndef MAX_NEST
#define MAX_NEST 32
#endif
#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN 1024
#endif
#define DEFAULT_NEST_SEPARATOR '.'
#define MAX_IDXSTR_LEN 16

typedef struct free_json_elems free_json_elems_t;
typedef struct free_json_elem free_json_elem_t;

struct free_json_elem {
	uint32_t path_len;
        json_string_t name;
        uint32_t elem_idx;
        uint8_t type;
};

struct free_json_elems {
	free_json_elem_t free_json_elems[MAX_NEST];
	uint32_t free_json_elems_idx;
};

// convert string to integer
inline static const char *
streamtoll(
    const char *first,
    const char *last,
     long long *out)
{
	int negative = 0;
	long long result = 0;

	if (first != last) {
		if (*first == '-') {
			negative = 1;
			++first;
		} else if (*first == '+') {
			++first;
		}
	}
	for (; first != last && isdigit(*first); ++first) {
		result = (10 * result) + (*first - '0');
	}
	if (negative) {
		result *= -1;
	}
	*out = result;

	return first;
}

// convert string to floating point
inline static const char *
streamtod(
    const char *first,
    const char *last,
    double *out)
{
	// sign
	int negative = 0;
	double result = 0;
	double inv_base = 0.1f;
	int exponent_negative = 0;
	int exponent = 0;
	double power_of_ten = 10;

	if (first != last) {
		if (*first == '-') {
			negative = 1;
			++first;
		} else if (*first == '+') {
			++first;
		}
	}
	// integer part
	for (; first != last && isdigit(*first); ++first) {
		result = (10 * result) + (*first - '0');
	}
	// fraction part
	if (first != last && *first == '.') {
		++first;
		for (; first != last && isdigit(*first); ++first) {
			result += (*first - '0') * inv_base;
			inv_base *= 0.1f;
		}
	}
	if (negative) {
		result *= -1;
	}
	// exponent
	if (first != last && (*first == 'e' || *first == 'E')) {
		++first;
		if (*first == '-') {
			exponent_negative = 1;
			++first;
		} else if (*first == '+') {
			++first;
		}
		for (; first != last && isdigit(*first); ++first) {
			exponent = 10 * exponent + (*first - '0');
		}
	}
	if (exponent) {
		for (; exponent > 1; exponent--) {
			power_of_ten *= 10;
		}
		if (exponent_negative) {
			result /= power_of_ten;
		}
		else {
			result *= power_of_ten;
		}
	}
	*out = result;

	return first;
}

// convert hexadecimal string to unsigned integer
inline static const char *
hexstreamtoui(
    const char *first,
    const char *last,
    unsigned int *out)
{
	unsigned int result = 0;
	int digit;

	for (; first != last; ++first) {
		if (isdigit(*first)) {
			digit = *first - '0';
		} else if (*first >= 'a' && *first <= 'f') {
			digit = *first - 'a' + 10;
		} else if (*first >= 'A' && *first <= 'F') {
			digit = *first - 'A' + 10;
		} else {
			break;
		}
		result = 16 * result + digit;
	}
	if (out) {
		*out = result;
	}

	return first;
}

#define ERROR(it, desc)					\
do {							\
	const char *c;					\
	if (error_pos) {				\
		*error_pos = it; 			\
	}						\
	if (error_desc) {				\
		*error_desc = desc;			\
	}						\
	if (error_line) {				\
		*error_line = 1 - escaped_newlines;	\
		for (c = it; c != json; --c) {	\
			if (*c == '\n') ++*error_line;	\
		}					\
	}						\
	return 1;					\
} while(0)

#define CHECK_TOP() \
    if (!top) {ERROR(it, "Unexpected character");}

#define IS_END() \
    (json_size > 0 && (it - json) > json_size)

#define GET_PATH(pos) 											\
do {													\
	if (name.ptr) {											\
		if (path_len + 1 + name.len >= MAX_PATH_LEN) {						\
			ERROR((pos), "path is too long");						\
		}											\
		if (path_len != 0) {									\
			path[path_len] = nsep;								\
			path_len += 1;									\
		}											\
		memcpy(&path[path_len], name.ptr, name.len);						\
		path_len += name.len;									\
		path[path_len] = '\0';									\
	} else {											\
		if (path_len + 1 + MAX_IDXSTR_LEN >= MAX_PATH_LEN) {					\
			ERROR((pos), "path is too long");						\
		}											\
		if (path_len != 0) {									\
			path[path_len] = nsep;								\
			path_len += 1;									\
		}											\
		idxstr_len = snprintf(&path[path_len], MAX_PATH_LEN - path_len, "%d", top->elem_idx);	\
		if (idxstr_len < 1) {									\
			ERROR((pos), "snprintf error");							\
		}											\
		path_len += idxstr_len;									\
	}												\
} while (0)

int
json_finder_find(
    json_elem_t *target,
    const char *json,
    ssize_t json_size,
    const char *key,
    char nest_sep,
    char const **error_pos,
    char const **error_desc,
    int *error_line)
{
	free_json_elem_t *root = NULL;
	free_json_elem_t *top = NULL;
	free_json_elem_t *free_elem = NULL;
	json_string_t name = {
		.ptr = NULL,
		.len = 0,
	};
	free_json_elems_t free_json_elems = {
		.free_json_elems_idx = 0,
	};
	int escaped_newlines = 0;
	const char *it;
	const char *first;
	char nsep;
        uint8_t type;
	union {
		long long ll;
		double d;
	} val;
	char path[MAX_PATH_LEN];
	uint32_t path_len = 0;
	uint32_t prev_path_len;
	int8_t idxstr_len;

	it = json;
	if (nest_sep) {
		nsep = nest_sep;
	} else {
		nsep = DEFAULT_NEST_SEPARATOR;
	}
	while (*it && !IS_END()) {
		switch (*it) {
		case '{':
		case '[':
			// create new value
			free_json_elems.free_json_elems_idx++;
			if (free_json_elems.free_json_elems_idx > MAX_NEST) {
				ERROR(it, "Too many nest");
			} else {
				free_elem = &free_json_elems.free_json_elems[free_json_elems.free_json_elems_idx - 1];
			}
			if (free_elem == NULL) {
				ERROR(it, "Could not get free element");
			}
			// name
			free_elem->name.ptr = name.ptr;
			free_elem->name.len = name.len;
			free_elem->elem_idx = 0;
			// type
			free_elem->type = (*it == '{') ? JSON_OBJECT : JSON_ARRAY;
			// set top and root
			if (top) {
				GET_PATH(it);
				free_elem->path_len = path_len;
				top->elem_idx++;
			} else if (!root) {
				path[0] = '\0';
				path_len = free_elem->path_len = 0;
				root = free_elem;
			} else {
				ERROR(it, "Second root. Only one root allowed");
			}
			// skip open character
			++it;
			top = free_elem;
			name.ptr = NULL;
			break;
		case '}':
		case ']':
			if (!top || top->type != ((*it == '}') ? JSON_OBJECT : JSON_ARRAY)) {
				ERROR(it, "Mismatch closing brace/bracket");
			}
			// skip close character
			++it;
			// set top
			free_json_elems.free_json_elems_idx--;
			if (free_json_elems.free_json_elems_idx > 0) {
				top =  &free_json_elems.free_json_elems[free_json_elems.free_json_elems_idx - 1];
				path_len = top->path_len;
			} else {
				top = NULL;
			}
			break;
		case ':':
			if (!top || top->type != JSON_OBJECT)
			{
				ERROR(it, "Unexpected character");
			}
			++it;
			break;
		case ',':
			CHECK_TOP();
			++it;
			break;
		case '"':
			CHECK_TOP();
			// skip '"' character
			++it;
			first = it;
			while (*it) {
				if ((unsigned char)*it < '\x20') {
					ERROR(first, "Control characters not allowed in strings");
				} else if (*it == '\\') {
					switch (*(it + 1)) {
					case '"':
						break;
					case '\\':
						break;
					case '/':
						break;
					case 'b':
						break;
					case 'f':
						break;
					case 'n':
						++escaped_newlines;
						break;
					case 'r':
						break;
					case 't':
						break;
					case 'u':
						if (hexstreamtoui(it + 2, it + 6, NULL) != it + 6) {
							ERROR(it, "Bad unicode codepoint");
						}
						it += 4;
						break;
					default:
						ERROR(first, "Unrecognized escape sequence");
					}
					it += 2;
				} else if (*it == '"') {
					++it;
					break;
				} else {
					++it;
				}
				if (IS_END()) {
					ERROR(it, "Bad string");
				}
			}
			if (!name.ptr && top->type == JSON_OBJECT) {
				// field name in element
				name.ptr = first;
				name.len = (it - first) - 1;
			} else {
				// new string value
				prev_path_len = path_len;
				GET_PATH(it);
				if (strcmp(path, key) == 0) {
					target->name.ptr = name.ptr;
					target->name.len = name.len;
					target->elem_idx = top->elem_idx;
					target->type = JSON_STRING;
					target->value.s.ptr = first;
					target->value.s.len = (it - first) - 1;
					return 0;	
				}
				path_len = prev_path_len;
				top->elem_idx++;
				name.ptr = NULL;
			}
			break;
		case 'n':
		case 't':
		case 'f':
			CHECK_TOP();
			// new boolean value or new null value
			if (*it == 'n' && *(it + 1) == 'u' && *(it + 2) == 'l' && *(it + 3) == 'l') {
				// null
				prev_path_len = path_len;
				GET_PATH(it);
				if (strcmp(path, key) == 0) {
					target->name.ptr = name.ptr;
					target->name.len = name.len;
					target->elem_idx = top->elem_idx;
					target->type = JSON_NULL;
					return 0;
				}
				path_len = prev_path_len;
				it += 4;
			} else if (*it == 't' && *(it + 1) == 'r' && *(it + 2) == 'u' && *(it + 3) == 'e') {
				// true
				prev_path_len = path_len;
				GET_PATH(it);
				if (strcmp(path, key) == 0) {
					target->name.ptr = name.ptr;
					target->name.len = name.len;
					target->elem_idx = top->elem_idx;
					target->type = JSON_BOOL;
					target->value.b = 1;
					return 0;
				}
				path_len = prev_path_len;
				it += 4;
			} else if (*it == 'f' && *(it + 1) == 'a' && *(it + 2) == 'l' && *(it + 3) == 's' && *(it + 4) == 'e') {
				// false
				prev_path_len = path_len;
				GET_PATH(it);
				if (strcmp(path, key) == 0) {
					target->name.ptr = name.ptr;
					target->name.len = name.len;
					target->elem_idx = top->elem_idx;
					target->type = JSON_BOOL;
					target->value.b = 0;
					return 0;
				}
				path_len = prev_path_len;
				it += 5;
			} else {
				ERROR(it, "Unknown identifier");
			}
			top->elem_idx++;
			name.ptr = NULL;
			break;
		case '-':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			CHECK_TOP();
			// new integer value or new double value
			type = JSON_INTEGER;
			first = it;
			while (*it != '\x20' && *it != '\x9' && *it != '\xD' && *it != '\xA' && *it != ',' && *it != ']' && *it != '}') {
				if (*it == '.' || *it == 'e' || *it == 'E')
				{
					type = JSON_DOUBLE;
				}
				++it;
				if (IS_END()) {
					ERROR(it, "Bad number");
				}
			}
			if (type == JSON_INTEGER) {
				if (streamtoll(first, it, &val.ll) != it) {
					ERROR(first, "Bad integer number");
				}
				prev_path_len = path_len;
				GET_PATH(first);
				if (strcmp(path, key) == 0) {
					target->name.ptr = name.ptr;
					target->name.len = name.len;
					target->elem_idx = top->elem_idx;
					target->type = JSON_INTEGER;
					target->value.ll = val.ll;
					return 0;
				}
				path_len = prev_path_len;
			} else {
				if (streamtod(first, it, &val.d) != it) {
					ERROR(first, "Bad float number");
				}
				prev_path_len = path_len;
				GET_PATH(first);
				if (strcmp(path, key) == 0) {
					target->name.ptr = name.ptr;
					target->name.len = name.len;
					target->elem_idx = top->elem_idx;
					target->type = JSON_DOUBLE;
					target->value.d = val.d;
					return 0;
				}
				path_len = prev_path_len;
			}
			top->elem_idx++;
			name.ptr = NULL;
			break;
		default:
			ERROR(it, "Unexpected character");
		}
		// skip white space
		while (*it == '\x20' || *it == '\x9' || *it == '\xD' || *it == '\xA') {
			++it;
			if (IS_END()) {
				break;
			}
		}
	}
	if (top) {
		ERROR(it, "Not all elements/arrays have been properly closed");
	}

	/* not found */
	return -1;
}

int
json_finder_unescape_strdup(
    char **unescape_str,
    size_t *unescape_str_size,
    json_string_t *str) {
	char *first;
	char *it;
	char *last;
	unsigned int codepoint;
	
	if ((first = malloc(str->len + 1)) == NULL) {
		return 1;
	}
	memcpy(first, str->ptr, str->len);
	first[str->len] = '\0';
	it = first;
	last = it;
	while (*it) {
		if (*it == '\\') {
			switch (*(it + 1)) {
			case '"':
				*last = '"';
				break;
			case '\\':
				*last = '\\';
				break;
			case '/':
				*last = '/';
				break;
			case 'b':
				*last = '\b';
				break;
			case 'f':
				*last = '\f';
				break;
			case 'n':
				*last = '\n';
				break;
			case 'r':
				*last = '\r';
				break;
			case 't':
				*last = '\t';
				break;
			case 'u':
				if (hexstreamtoui(it + 2, it + 6, &codepoint) != it + 6) {
					free(first);
					return 1;
				}
				if (codepoint <= 0x7F) {
					*last = (char)codepoint;
				} else if (codepoint <= 0x7FF) {
					*last++ = (char)(0xC0 | (codepoint >> 6));
					*last = (char)(0x80 | (codepoint & 0x3F));
				} else if (codepoint <= 0xFFFF) {
					*last++ = (char)(0xE0 | (codepoint >> 12));
					*last++ = (char)(0x80 | ((codepoint >> 6) & 0x3F));
					*last = (char)(0x80 | (codepoint & 0x3F));
				}
				it += 4;
				break;
			default:
				free(first);
				return 1;
			}
			++last;
			it += 2;
		} else {
			*last++ = *it++;
		}
	}
	*last++ = '\0';
	*unescape_str = first;
	*unescape_str_size = last - first;

	return 0;
}

int
json_finder_minimize(
    char **json_min,
    size_t *json_min_size,
    const char *json)
{
	char *first;
	char *it;
	char *last;
	
	if ((first = strdup(json)) == NULL) {
		return 1;
	}
	it = first;
	last = it;
	while (*it) {
		if (*it == '"') {
			*last++ = *it++;
			while (*it) {
				if (*it == '\\' && *(it + 1) == '"') {
					*last++ = *it++;
					*last++ = *it++;
				} else if (*it == '"') {
					*last++ = *it++;
					break;
				} else {
					*last++ = *it++;
				}
			}
		} else if (*it == '\x20' || *it == '\x9' || *it == '\xD' || *it == '\xA') {
			++it;
		} else {
			*last++ = *it++;
		}
	}
	*last++ = '\0';
	*json_min = first;
	*json_min_size = last - first;

	return 0;
}

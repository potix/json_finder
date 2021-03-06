#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "json_finder.h"

#ifndef INIT_NEST_MAX
#define INIT_NEST_MAX 32
#endif
#define NEST_SEPARATOR '.'
#define MAX_IDXSTR_LEN 16
#define ALIGN_SIZE(size) ((size) + 8 - ((size) & 7))

typedef struct nest_json_elems nest_json_elems_t;
typedef struct nest_json_elem nest_json_elem_t;

struct nest_json_elem {
	uint32_t elem_idx;
	uint8_t type;
};

struct nest_json_elems {
	nest_json_elem_t *nest_json_elems;
	uint32_t nest_json_elems_count;
	uint32_t nest_json_elems_idx;
};


// true if character represent a digit
#define IS_DIGIT(c) (c >= '0' && c <= '9')

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
	for (; first != last && IS_DIGIT(*first); ++first) {
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
	for (; first != last && IS_DIGIT(*first); ++first) {
		result = (10 * result) + (*first - '0');
	}
	// fraction part
	if (first != last && *first == '.') {
		++first;
		for (; first != last && IS_DIGIT(*first); ++first) {
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
		for (; first != last && IS_DIGIT(*first); ++first) {
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
		if (IS_DIGIT(*first)) {
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
		for (c = it; c != json; --c) {		\
			if (*c == '\n') ++*error_line;	\
		}					\
	}						\
	free(mem);					\
	return 1;					\
} while(0)

#define CHECK_TOP() \
	if (!top) {ERROR(it, "Unexpected character");}

#define IS_FIND_END() \
	(json_len > 0 && (it - json) >= json_len)

#define KEY_STRCHAR()								\
do {										\
	last_key = NULL;							\
	it_key = sep_key_start;							\
	while (*it_key != '\0' && *it_key != NEST_SEPARATOR) {			\
		if (*it_key == '\\') {						\
			if (*(it_key + 1) == '.') {				\
				if (!last_key) {				\
					last_key = it_key;			\
				}						\
				it_key++;					\
				*last_key++ = *it_key++;			\
			} else {						\
				if (last_key) {					\
					*last_key++ = *it_key++;		\
					*last_key++ = *it_key++;		\
				} else {					\
					it_key += 2;				\
				}						\
			}							\
		} else {							\
			if (last_key) {						\
				*last_key++ = *it_key++;			\
			} else {						\
				it_key++;					\
			}							\
		}								\
	}									\
	if (last_key) {								\
		comp_sep_key_len = last_key - sep_key_start;			\
		sep_key_len = it_key - sep_key_start;				\
	} else {								\
		sep_key_len = comp_sep_key_len = it_key - sep_key_start;	\
	}									\
} while(0)

#define IDXTOSTR()					\
do {							\
        int i, v, r;					\
							\
	v = top->elem_idx;				\
        if (v == 0) {					\
		v = 1;					\
        }						\
        for (idxstr_len = 0; v != 0; idxstr_len++) {	\
                v /=  10;				\
        }						\
        idxstr[idxstr_len] = '\0';			\
	v = top->elem_idx;				\
        for (i = idxstr_len - 1; i >= 0; i--) {		\
                r  =  v % 10;				\
                idxstr[i] = r + '0';			\
                v /= 10;				\
        }						\
} while(0)

int
json_finder_find(
    json_elem_t *target,
    const char *json,
    ssize_t json_len,
    const char *key,
    ssize_t key_len,
    char const **error_pos,
    char const **error_desc,
    int *error_line)
{
	nest_json_elem_t *root = NULL;
	nest_json_elem_t *top = NULL;
	nest_json_elem_t *nest_elem = NULL;
	json_string_t name = {
		.ptr = NULL,
		.len = 0,
	};
	nest_json_elems_t nest_json_elems = {
		.nest_json_elems = NULL,
		.nest_json_elems_count = 0,
		.nest_json_elems_idx = 0,
	};
	int escaped_newlines = 0;
	const char *it;
	const char *first;
        uint8_t type;
	union {
		long long ll;
		double d;
	} val;
	char idxstr[MAX_IDXSTR_LEN];
	char *copy_key = NULL;
	uint32_t key_align_size;
	char *sep_key_start;
	uint32_t sep_key_start_off;
	uint32_t comp_sep_key_len;
	uint32_t sep_key_len;
	int8_t idxstr_len;
	uint32_t skip = 0;
	void *mem, *old_mem;
	char *it_key;
	char *last_key;
	int offset;

	it = json;
	if (key_len < 0) {
            key_len = strlen(key);
        }
	key_align_size = ALIGN_SIZE(key_len + 1);
	mem = malloc(key_align_size + (sizeof(nest_json_elem_t) * INIT_NEST_MAX));
	if (mem == NULL) {
		ERROR(it, "Memory allocation error");
	}
	copy_key = mem,
	nest_json_elems.nest_json_elems = mem + key_align_size;
	nest_json_elems.nest_json_elems_count = INIT_NEST_MAX;
	memcpy(copy_key, key, key_len);
        copy_key[key_len] = '\0';
	sep_key_start = copy_key;
	KEY_STRCHAR();
	while (*it && !IS_FIND_END()) {
		switch (*it) {
		case '{':
		case '[':
			nest_elem = &nest_json_elems.nest_json_elems[nest_json_elems.nest_json_elems_idx];
			nest_json_elems.nest_json_elems_idx++;
			if (nest_json_elems.nest_json_elems_idx >= nest_json_elems.nest_json_elems_count) {
				sep_key_start_off = sep_key_start - copy_key;
				old_mem = mem;
				mem = realloc(
				    old_mem,
				    key_align_size + (sizeof(nest_json_elem_t) * nest_json_elems.nest_json_elems_count * 2));
				if (mem == NULL) {
					mem = old_mem;
					ERROR(it, "Memory allocation error");
				}
				copy_key = mem;
				sep_key_start = mem + sep_key_start_off;
				nest_json_elems.nest_json_elems = mem + key_align_size;
				nest_json_elems.nest_json_elems_count = nest_json_elems.nest_json_elems_count * 2;
				// adjust new pointer
                                nest_elem = &nest_json_elems.nest_json_elems[nest_json_elems.nest_json_elems_idx - 1];
                                offset = mem - old_mem;
                                top = (void *)top + offset;
                                root = (void *)root + offset;
			} 
			nest_elem->type = (*it == '{') ? JSON_OBJECT : JSON_ARRAY;
			nest_elem->elem_idx = 0;
			// skip open character
			++it;
			if (!root) {
				root = top = nest_elem;
				name.ptr = NULL;
				break;
			}
			if (!skip) {
				if (name.ptr != NULL) {
					if (name.len == comp_sep_key_len &&
					    strncmp(name.ptr, sep_key_start, comp_sep_key_len) == 0) {
						sep_key_start = sep_key_start + sep_key_len + 1;
						KEY_STRCHAR();
					} else {
						skip++;
					}
				} else {
					IDXTOSTR();
					if (idxstr_len == comp_sep_key_len &&
					    strncmp(idxstr, sep_key_start, comp_sep_key_len) == 0) {
						sep_key_start = sep_key_start + sep_key_len + 1;
						KEY_STRCHAR();
					} else {
						skip++;
					}
				}
			} else {
				skip++;
			}
			top->elem_idx++;	
			top = nest_elem;
			name.ptr = NULL;
			break;
		case '}':
		case ']':
			if (!top || top->type != ((*it == '}') ? JSON_OBJECT : JSON_ARRAY)) {
				ERROR(it, "Mismatch closing brace/bracket");
			}
			// skip close character
			++it;
                        if (skip) {
				skip--;
			}
			nest_json_elems.nest_json_elems_idx--;
			if (nest_json_elems.nest_json_elems_idx > 0) {
				top =  &nest_json_elems.nest_json_elems[nest_json_elems.nest_json_elems_idx - 1];
			} else {
				top = NULL;
			}
			break;
		case ':':
			if (!top || top->type != JSON_OBJECT) {
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
				if (IS_FIND_END()) {
					ERROR(it, "Bad string");
				}
			}
			if (!name.ptr && top->type == JSON_OBJECT) {
				// field name in element
				name.ptr = first;
				name.len = (it - first) - 1;
			} else {
				// new string value
				if (!skip) {
					if (name.ptr != NULL) {
						if (name.len == comp_sep_key_len &&
						    strncmp(name.ptr, sep_key_start, comp_sep_key_len) == 0) {
							target->type = JSON_STRING;
							target->value.s.ptr = first;
							target->value.s.len = (it - first) - 1;
							free(mem);
							return 0;	
						}
					} else {
						IDXTOSTR();
						if (idxstr_len == comp_sep_key_len &&
						    strncmp(idxstr, sep_key_start, comp_sep_key_len) == 0) {
							target->type = JSON_STRING;
							target->value.s.ptr = first;
							target->value.s.len = (it - first) - 1;
							free(mem);
							return 0;	
						}
					}
				}
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
				if (!skip) {
					if (name.ptr != NULL) {
						if (name.len == comp_sep_key_len &&
						    strncmp(name.ptr, sep_key_start, comp_sep_key_len) == 0) {
							target->type = JSON_NULL;
							free(mem);
							return 0;	
						}
					} else {
						IDXTOSTR();
						if (idxstr_len == comp_sep_key_len &&
						    strncmp(idxstr, sep_key_start, comp_sep_key_len) == 0) {
							target->type = JSON_NULL;
							free(mem);
							return 0;	
						}
					}
				}
				it += 4;
			} else if (*it == 't' && *(it + 1) == 'r' && *(it + 2) == 'u' && *(it + 3) == 'e') {
				// true
				if (!skip) {
					if (name.ptr != NULL) {
						if (name.len == comp_sep_key_len &&
						    strncmp(name.ptr, sep_key_start, comp_sep_key_len) == 0) {
							target->type = JSON_BOOL;
							target->value.b = 1;
							free(mem);
							return 0;	
						}
					} else {
						IDXTOSTR();
						if (idxstr_len == comp_sep_key_len &&
						    strncmp(idxstr, sep_key_start, comp_sep_key_len) == 0) {
							target->type = JSON_BOOL;
							target->value.b = 1;
							free(mem);
							return 0;	
						}
					}
				}
				it += 4;
			} else if (*it == 'f' && *(it + 1) == 'a' && *(it + 2) == 'l' && *(it + 3) == 's' && *(it + 4) == 'e') {
				// false
				if (!skip) {
					if (name.ptr != NULL) {
						if (name.len == comp_sep_key_len &&
						    strncmp(name.ptr, sep_key_start, comp_sep_key_len) == 0) {
							target->type = JSON_BOOL;
							target->value.b = 0;
							free(mem);
							return 0;	
						}
					} else {
						IDXTOSTR();
						if (idxstr_len == comp_sep_key_len &&
						    strncmp(idxstr, sep_key_start, comp_sep_key_len) == 0) {
							target->type = JSON_BOOL;
							target->value.b = 0;
							free(mem);
							return 0;	
						}
					}
				}
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
				if (IS_FIND_END()) {
					ERROR(it, "Bad number");
				}
			}
			if (type == JSON_INTEGER) {
				if (streamtoll(first, it, &val.ll) != it) {
					ERROR(first, "Bad integer number");
				}
				if (!skip) {
					if (name.ptr != NULL) {
						if (name.len == comp_sep_key_len &&
						    strncmp(name.ptr, sep_key_start, comp_sep_key_len) == 0) {
							target->type = JSON_INTEGER;
							target->value.ll.v = val.ll;
							target->value.ll.s.ptr = first;
							target->value.ll.s.len = it - first;
							free(mem);
							return 0;	
						}
					} else {
						IDXTOSTR();
						if (idxstr_len == comp_sep_key_len &&
						    strncmp(idxstr, sep_key_start, comp_sep_key_len) == 0) {
							target->type = JSON_INTEGER;
							target->value.ll.v = val.ll;
							target->value.ll.s.ptr = first;
							target->value.ll.s.len = it - first;
							free(mem);
							return 0;	
						}
					}
				}
			} else {
				if (streamtod(first, it, &val.d) != it) {
					ERROR(first, "Bad float number");
				}
				if (!skip) {
					if (name.ptr != NULL) {
						if (name.len == comp_sep_key_len &&
						    strncmp(name.ptr, sep_key_start, comp_sep_key_len) == 0) {
							target->type = JSON_DOUBLE;
							target->value.d.v = val.d;
							target->value.d.s.ptr = first;
							target->value.d.s.len = it - first;
							free(mem);
							return 0;	
						}
					} else {
						IDXTOSTR();
						if (idxstr_len == comp_sep_key_len &&
						    strncmp(idxstr, sep_key_start, comp_sep_key_len) == 0) {
							target->type = JSON_DOUBLE;
							target->value.d.v = val.d;
							target->value.d.s.ptr = first;
							target->value.d.s.len = it - first;
							free(mem);
							return 0;	
						}
					}
				}
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
			if (IS_FIND_END()) {
				break;
			}
		}
	}
	if (top) {
		ERROR(it, "Not all elements/arrays have been properly closed");
	}

	/* not found */
	free(mem);
	return -1;
}

#define IS_STRDUP_END() \
	((it - str->ptr) >= str->len)
int
json_finder_unescape_strdup(
    char **unescape_str,
    size_t *unescape_str_size,
    json_string_t *str) {
	const char *it;
	char *first;
	char *last;
	unsigned int codepoint;
	
	first = malloc(str->len + 1);
	if (first == NULL) {
		return 1;
	}
	it = str->ptr;
	last = first;
	while (!IS_STRDUP_END()) {
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


#define IS_MIN_END() \
        (json_len > 0 && (it - json) >= json_len)

int
json_finder_minimize(
    char **json_min,
    ssize_t *json_min_size,
    const char *json,
    ssize_t json_len)
{
	const char *it;
	char *first;
	char *last;
	
	if (json_len < 0) {
		json_len = strlen(json);
	}
	first = malloc((size_t)json_len);
	if (first == NULL) {
		return 1;
	}
	it = json;
	last = first;
	while (*it && !IS_MIN_END()) {
		if (*it == '"') {
			*last++ = *it++;
			while (*it) {
				if (*it == '\\' && (*(it + 1) == '"' || *(it + 1) == '\\')) {
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "json_finder.h"

#define JSON_TEST1 \
    "{\"a\":\"1\", \"hoge\":10, \"fuga\":\"aaa\", \"oo\": null, \"b\":{\"c\": \"???\", \"piyo\":false, \"l\":0.5, \"o\":-10000, \"t\":[\"t\", 5]}, \"iii\": { \"ppp\": 1, \"qqq\" : [ 10, 11 ] }, \"###\": 2, \"nnn\" : { \"ooo\" : { \"rrr\" : 3}}}"

#define KEY(str) \
     (str), sizeof((str))

int main(void) {
	int result;
	json_elem_t elem;
	const char *desc;
#ifndef PROF
	char *str;
	size_t str_size;
#endif

#ifdef PROF
	int i;

	for (i = 0; i < 1000000; i++) {
#endif

	result = json_finder_find(&elem, JSON_TEST1, strlen(JSON_TEST1), KEY("a"), 0, NULL, &desc, NULL);
#ifndef PROF
	if (result == 1) {
                printf("parse error: %s\n", desc);
		return 1;
        } else if (result == -1) {
                printf("not found\n");
		return 1;
	}
	if (elem.type != JSON_STRING) {
                printf("parse error\n");
		return 1;
	}
	if (json_finder_unescape_strdup(&str, &str_size, &elem.value.s)) {
                printf("parse error\n");
		return 1;
	}
	printf("a: > %s\n", str); 
#endif

	result  = json_finder_find(&elem, JSON_TEST1, strlen(JSON_TEST1), KEY("hoge"), 0, NULL, &desc, NULL);
#ifndef PROF
	if (result == 1) {
                printf("parse error: %s\n", desc);
		return 1;
        } else if (result == -1) {
                printf("not found\n");
		return 1;
	}
	if (elem.type != JSON_INTEGER) {
                printf("parse error\n");
		return 1;
	}
	printf("hoge: > %lld\n", elem.value.ll); 
#endif

	result = json_finder_find(&elem, JSON_TEST1, strlen(JSON_TEST1), KEY("fuga"), 0, NULL, &desc, NULL);
#ifndef PROF
	if (result == 1) {
                printf("parse error: %s\n", desc);
		return 1;
        } else if (result == -1) {
                printf("not found\n");
		return 1;
	}
	if (elem.type != JSON_STRING) {
                printf("parse error\n");
		return 1;
	}
	if (json_finder_unescape_strdup(&str, &str_size, &elem.value.s)) {
                printf("parse error\n");
		return 1;
	}
	printf("fuga: > %s\n", str); 
#endif

	result = json_finder_find(&elem, JSON_TEST1, strlen(JSON_TEST1), KEY("oo"), 0, NULL, &desc, NULL);
#ifndef PROF
	if (result == 1) {
                printf("parse error: %s\n", desc);
		return 1;
        } else if (result == -1) {
                printf("not found\n");
		return 1;
	}
	if (elem.type != JSON_NULL) {
                printf("parse error\n");
		return 1;
	}
	printf("oo: > null\n"); 
#endif

	result = json_finder_find(&elem, JSON_TEST1, strlen(JSON_TEST1), KEY("b.c"), 0, NULL, &desc, NULL);
#ifndef PROF
	if (result == 1) {
                printf("parse error: %s\n", desc);
		return 1;
        } else if (result == -1) {
                printf("not found\n");
		return 1;
	}
	if (elem.type != JSON_STRING) {
                printf("parse error\n");
		return 1;
	}
	if (json_finder_unescape_strdup(&str, &str_size, &elem.value.s)) {
                printf("parse error\n");
		return 1;
	}
	printf("b.c: > %s\n", str); 
#endif

	result = json_finder_find(&elem, JSON_TEST1, strlen(JSON_TEST1), KEY("b.piyo"), 0, NULL, &desc, NULL);
#ifndef PROF
	if (result == 1) {
                printf("parse error: %s\n", desc);
		return 1;
        } else if (result == -1) {
                printf("not found\n");
		return 1;
	}
	if (elem.type != JSON_BOOL) {
                printf("parse error\n");
		return 1;
	}
	printf("b.piyo: > %d\n", elem.value.b); 
#endif

	result = json_finder_find(&elem, JSON_TEST1, strlen(JSON_TEST1), KEY("b.l"), 0, NULL, &desc, NULL);
#ifndef PROF
	if (result == 1) {
                printf("parse error: %s\n", desc);
		return 1;
        } else if (result == -1) {
                printf("not found\n");
		return 1;
	}
	if (elem.type != JSON_DOUBLE) {
                printf("parse error\n");
		return 1;
	}
	printf("b.l: > %lf\n", elem.value.d); 
#endif

	result = json_finder_find(&elem, JSON_TEST1, strlen(JSON_TEST1), KEY("b.o"), 0, NULL, &desc, NULL);
#ifndef PROF
	if (result == 1) {
                printf("parse error: %s\n", desc);
		return 1;
        } else if (result == -1) {
                printf("not found\n");
		return 1;
	}
	if (elem.type != JSON_INTEGER) {
                printf("parse error\n");
		return 1;
	}
	printf("b.o: > %lld\n", elem.value.ll); 
#endif

	result = json_finder_find(&elem, JSON_TEST1, strlen(JSON_TEST1), KEY("b.t.0"), 0, NULL, &desc, NULL);
#ifndef PROF
	if (result == 1) {
                printf("parse error: %s\n", desc);
		return 1;
        } else if (result == -1) {
                printf("not found\n");
		return 1;
	}
	if (elem.type != JSON_STRING) {
                printf("parse error\n");
		return 1;
	}
	if (json_finder_unescape_strdup(&str, &str_size, &elem.value.s)) {
                printf("parse error\n");
		return 1;
	}
	printf("b.t.0: > %s\n", str); 
#endif

	result = json_finder_find(&elem, JSON_TEST1, strlen(JSON_TEST1), KEY("b.t.1"), 0, NULL, &desc, NULL);
#ifndef PROF
	if (result == 1) {
                printf("parse error: %s\n", desc);
		return 1;
        } else if (result == -1) {
                printf("not found\n");
		return 1;
	}
	if (elem.type != JSON_INTEGER) {
                printf("parse error\n");
		return 1;
	}
	printf("b.t.1: > %lld\n", elem.value.ll); 
#endif

	result = json_finder_find(&elem, JSON_TEST1, strlen(JSON_TEST1), KEY("b.mmm"), 0, NULL, &desc, NULL);
#ifndef PROF
	if (result == 1) {
                printf("parse error: %s\n", desc);
		return 1;
        } else if (result != -1) {
                printf("parse error: %s\n", desc);
		return 1;
	} 
#endif

	result = json_finder_find(&elem, JSON_TEST1, strlen(JSON_TEST1), KEY("nnn.ooo.rrr"), 0, NULL, &desc, NULL);
#ifndef PROF
	if (result == 1) {
                printf("parse error: %s\n", desc);
		return 1;
        } else if (result == -1) {
                printf("not found\n");
		return 1;
	}
	if (elem.type != JSON_INTEGER) {
                printf("parse error\n");
		return 1;
	}
	printf("nnn.ooo.rrr: > %lld\n", elem.value.ll); 
#endif

#ifdef PROF
	}
#endif
	return 0;
}

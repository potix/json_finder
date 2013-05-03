#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "json_finder.h"

#define JSON_TEST1 "{\"a\":\"1\", \"hoge\":10, \"fuga\":\"aaa\", \"oo\": null, \"b\":{\"c\": \"???\", \"piyo\":false, \"l\":0.5, \"o\":-10000, \"t\":[\"t\", 5]}}"

int main(void) {
	int result;
	json_elem_t elem;
	const char *desc;
#ifdef PROF
	int i;

	for (i = 0; i < 1000000; i++) {
#endif

	result = json_finder_find(&elem, JSON_TEST1, strlen(JSON_TEST1), "a",  0, NULL, &desc, NULL);
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
	printf("a: > %s\n", json_finder_unescape_strdup(&elem.value.s)); 
#endif

	result  = json_finder_find(&elem, JSON_TEST1, strlen(JSON_TEST1), "hoge", 0, NULL, &desc, NULL);
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

	result = json_finder_find(&elem, JSON_TEST1, strlen(JSON_TEST1), "fuga", 0, NULL, &desc, NULL);
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
	printf("fuga: > %s\n", json_finder_unescape_strdup(&elem.value.s)); 
#endif

	result = json_finder_find(&elem, JSON_TEST1, strlen(JSON_TEST1), "oo", 0, NULL, &desc, NULL);
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

	result = json_finder_find(&elem, JSON_TEST1, strlen(JSON_TEST1), "b.c", 0, NULL, &desc, NULL);
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
	printf("b.c: > %s\n", json_finder_unescape_strdup(&elem.value.s)); 
#endif

	result = json_finder_find(&elem, JSON_TEST1, strlen(JSON_TEST1), "b.piyo", 0, NULL, &desc, NULL);
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

	result = json_finder_find(&elem, JSON_TEST1, strlen(JSON_TEST1), "b.l", 0, NULL, &desc, NULL);
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

	result = json_finder_find(&elem, JSON_TEST1, strlen(JSON_TEST1), "b.o", 0, NULL, &desc, NULL);
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

	result = json_finder_find(&elem, JSON_TEST1, strlen(JSON_TEST1), "b.t.0", 0, NULL, &desc, NULL);
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
	printf("b.t.0: > %s\n", json_finder_unescape_strdup(&elem.value.s)); 
#endif

	result = json_finder_find(&elem, JSON_TEST1, strlen(JSON_TEST1), "b.t.1", 0, NULL, &desc, NULL);
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

	result = json_finder_find(&elem, JSON_TEST1, strlen(JSON_TEST1), "b.mmm", 0, NULL, &desc, NULL);
#ifndef PROF
	if (result == 1) {
                printf("parse error: %s\n", desc);
		return 1;
        } else if (result != -1) {
                printf("parse error: %s\n", desc);
		return 1;
	} 
#endif

#ifdef PROF
	}
#endif
	return 0;
}

OS=$(shell uname)
CFLAGS=-Wall -O3 -fPIC -I.
ifeq ($(OS), Darwin)
  LDFLAGS=-bundle
else
  LDFLAGS=-shared
endif

SQLITE_JSON_FINDER_SHNAME=sqlite_json_finder.so.0.0.0
SQLITE_JSON_FINDER_SRCS=json_finder.c sqlite_json_finder.c
SQLITE_JSON_FINDER_OBJS=$(SQLITE_JSON_FINDER_SRCS:%.c=%.o)
MYSQL_JSON_FINDER_SHNAME=mysql_json_finder.so.0.0.0
MYSQL_JSON_FINDER_SRCS=json_finder.c mysql_json_finder.c
MYSQL_JSON_FINDER_OBJS=$(MYSQL_JSON_FINDER_SRCS:%.c=%.o)
JSON_FINDER_TEST=json_finder_test
JSON_FINDER_PROF=json_finder_prof

all: sqlite_json_finder mysql_json_finder
test: $(JSON_FINDER_TEST)
prof: $(JSON_FINDER_PROF)
clean: json_finder_clean

sqlite_json_finder: $(SQLITE_JSON_FINDER_OBJS)
	gcc $(CFLAGS) $(LDFLAGS) -o $(SQLITE_JSON_FINDER_SHNAME) $? 
mysql_json_finder: $(MYSQL_JSON_FINDER_OBJS)
	gcc $(CFLAGS) $(LDFLAGS) -o $(MYSQL_JSON_FINDER_SHNAME) $?
json_finder.o: json_finder.h
	gcc $(CFLAGS) -c $*.c
sqlite_json_finder.o: json_finder.h
	gcc $(CFLAGS) -c $*.c
mysql_json_finder.o: json_finder.h
	gcc $(CFLAGS) -c $*.c

$(JSON_FINDER_TEST):
	gcc $(CFLAGS) -o $@ json_finder.c json_finder_test.c
$(JSON_FINDER_PROF):
	gcc $(CFLAGS) -pg -DPROF -o $@ json_finder.c json_finder_test.c

json_finder_clean:
	rm  -rf *.o $(SQLITE_JSON_FINDER_SHNAME) $(MYSQL_JSON_FINDER_SHNAME) $(JSON_FINDER_TEST) $(JSON_FINDER_PROF)

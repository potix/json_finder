OS=$(shell uname)
CFLAGS=-Wall -O3 -fPIC -I.
ifeq ($(OS), Linux)
  LDFLAGS=-shared
else
  LDFLAGS=-bundle
endif
FINDER_SHNAME=sqlite_json_finder.so.0.0.0
FINDER_SRCS=json_finder.c sqlite_json_finder.c
FINDER_OBJS=$(FINDER_SRCS:%.c=%.o)
FINDER_TEST=json_finder_test
FINDER_PROF=json_finder_prof

all: sqlite_json_finder
test: $(FINDER_TEST)
prof: $(FINDER_PROF)
clean: finder_clean

sqlite_json_finder: $(FINDER_OBJS)
	gcc $(CFLAGS) $(FINDER_CFLAGS) $(LDFLAGS) $(FINDER_LDFLAGS) -o $(FINDER_SHNAME) $? $(FINDER_STATICLIBS)
json_finder.o: json_finder.h
	gcc $(CFLAGS) $(FINDER_CFLAGS) -c $*.c
sqlite_finder.o: json_finder.h
	gcc $(CFLAGS) $(FINDER_CFLAGS) -c $*.c
$(FINDER_TEST):
	gcc $(CFLAGS) -o $@ json_finder.c json_finder_test.c
$(FINDER_PROF):
	gcc $(CFLAGS) -pg -DPROF -o $@ json_finder.c json_finder_test.c
finder_clean:
	rm  -rf *.o $(FINDER_SHNAME) $(FINDER_TEST) $(FINDER_PROF)

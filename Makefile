CFLAGS=-std=c11 -Wall -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

ghcc: $(OBJS)
	$(CC) -o ghcc $(OBJS) $(LDFLAGS)

$(OBJS): ghcc.h

test: ghcc
	./test.sh

clean:
	rm -f ghcc *.o *~ tmp*

.PHONY: test clean

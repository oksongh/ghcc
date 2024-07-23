CFLAGS=-std=c11 -g -static

ghcc: ghcc.c

test: ghcc
	./test.sh

clean:
	rm -f ghcc *.o *~ tmp*

.PHONY: test clean

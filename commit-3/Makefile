CFLAGS=-std=c11 -g -fno-common

chibicc: main.o
	$(CC) -o chibicc main.o $(LDFLAGS)

test: chibicc
	python test.py

clean:
	rm -f chibicc *.yas *~ tmp* *.o

.PHONY: test clean
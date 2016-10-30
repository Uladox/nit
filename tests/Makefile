test: test.o ../libnit.a
	gcc -g -o test test.o munit/munit.c ../libnit.a

test.o: test.c ../ftree.h
	gcc -g -c test.c

../libnit.a:
	(cd .. && make)
clean:
	rm -f test test.o

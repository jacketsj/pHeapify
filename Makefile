test : test.cc pHeapify.cc pHeapify.h
	g++ -Wall -g -c -std=c++11 pHeapify.cc -o pHeapify.o -fopenmp
	g++ -Wall -g -std=c++11 test.cc pHeapify.o -o test -fopenmp

clean:
	rm -f pHeapify.o
	rm -f test

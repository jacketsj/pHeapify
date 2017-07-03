test : test.cc pHeapify.o
	g++ -Wall -g -std=c++11 -fopenmp test.cc pHeapify.o -o test

pHeapify : pHeapify.cc pHeapify.h
	g++ -Wall -g -c -std=c++11 -fopenmp pHeapify.cc -o pHeapify.o

clean:
	rm -f pHeapify.o
	rm -f test

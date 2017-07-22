all : test pHeapify.pdf

pHeapify.pdf : pHeapify.tex
	pdflatex -halt-on-error pHeapify.tex

test : test.cc pHeapify.o
	g++ -Wall -g -std=c++11 test.cc pHeapify.o -o test -fopenmp

pHeapify.o : pHeapify.cc pHeapify.h
	g++ -Wall -g -c -std=c++11 pHeapify.cc -o pHeapify.o -fopenmp

clean :
	rm -f pHeapify.o
	rm -f pHeapify.aux
	rm -f pHeapify.log
	rm -f test
	rm -f pHeapify.pdf

tidy :
	rm -f pHeapify.o
	rm -f pHeapify.aux
	rm -f pHeapify.log

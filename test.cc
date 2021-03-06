#include <iostream>
#include <string>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include "pHeapify.h"

void runTests();
void printResults(int *heap, int *notHeap, int n);
int* generateRandomDistinct(int n, int max, int min);
bool verifyAnswer(int* in, int* out, int n);
bool verifyHeap(int* out, int n);
bool verifyPresence(int* in, int* out, int n);
bool contains(int c, int* arr, int n);

int main(int argc, char* argv[])
{
	if (argc > 1)
	{
		int *heap, *notHeap = new int[argc-1];
		//std::string inp(*argv+1);
		//std::string num;
		//std::stringstream todo(inp);
		for (int i = 0; i < argc-1/* && todo >> num*/; ++i)
		{
			notHeap[i] = std::atoi(argv[1+i]);
		}
		heap = pHeapify(notHeap, argc-1);
		printResults(heap, notHeap, argc-1);

		delete[] heap;
		delete[] notHeap;
	}
	else
	{
		runTests();
	}
}

void runTests()
{
	std::srand(std::time(NULL));
	int num = 10;
	for (int i = 0; i < num; ++i)
	{
		int size = 10; //OpenMP cannot create enough threads for more
		int n = rand() % size;
		int* notHeap = generateRandomDistinct(n, size * 2, 0);
		int* heap = pHeapify(notHeap, n);
		std::string eq = "=";
		if (verifyAnswer(notHeap, heap, n))
		{
			std::cout << "Test passed:";
		}
		else
		{
			std::cout << "Test failed:";
			eq = "!=";
		}
		printResults(heap, notHeap, n);

		delete[] heap;
		delete[] notHeap;
	}
}

void printResults(int *heap, int *notHeap, int n)
{
	std::cout << " heapify({";
	for (int i = 0; i < n; ++i)
	{
		std::cout << notHeap[i] << (i == n-1 ? "" : ",");
	}
	std::cout << "})={";
	for (int i = 0; i < n; ++i)
	{
		std::cout << heap[i] << (i == n-1 ? "" : ",");
	}
	std::cout << "}" << std::endl;
}

bool verifyAnswer(int* in, int* out, int n)
{
	return (verifyHeap(out, n) && verifyPresence(in, out, n));
}

bool verifyHeap(int* out, int n)
{
	bool ret = true;
	for (int i = 0; i < n; ++i)
	{
		int lchild = 2*i+1, rchild = 2*i+2;
		ret = ret && (lchild >= n || out[i] > out[lchild])
			&& (rchild >= n || out[i] > out[rchild]);
	}
	return ret;
}

bool verifyPresence(int* in, int* out, int n)
{
	bool ret = true;
	for (int i = 0; i < n; ++i)
	{
		ret = ret && contains(in[i], out, n);
	}
	return ret;
}

bool contains(int c, int* arr, int n)
{
	for (int i = 0; i < n; ++i)
	{
		if (arr[i] == c)
		{
			return true;
		}
	}
	return false;
}

int* generateRandomDistinct(int n, int max, int min)
{
	if (min > max || max - min < n)
	{
		return NULL;
	}
	int c = 0;
	int* ins = new int[n]();
	if (min == max)
	{
		ins[0] = min;
		return ins;
	}
	while (c < n)
	{
		int p = (rand() % (max - min)) + min;
		bool good = true;
		for (int i = 0; i < c; ++i)
		{
			if (ins[i] == p)
			{
				good = false;
				break;
			}
		}
		if (good)
		{
			ins[c++] = p;
		}
	}
	return ins;
}

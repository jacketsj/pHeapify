#include <iostream>
#include <string>
#include <sstream>
#include <ctime>
#include "pHeapify.h"

void runTests();
int* generateRandomDistinct(int n, int max, int min);
bool verifyAnswer(int* in, int* out, int n);
bool verifyHeap(int* out, int n);
bool verifyPresence(int* in, int* out, int n);
bool contains(int c, int* arr, int n);

int main(int argc, char* argv[])
{
	if (argc > 1)
	{
		int* heap = new int[argc-1]();
		std::string inp(*argv+1);
		std::string num;
		std::stringstream todo(inp);
		for (int i = 0; todo >> num; ++i)
		{
			heap[i] = std::stoi(num);
		}
		pHeapify(heap, argc-1);
	}
	else
	{
		runTests();
	}
}

void runTests()
{
#if debug_sec
	std::srand(0);
#endif
#else
	std::srand(std::time);
#endif
	int num = 100;
#if debug_sec
	num = 5;
#endif
	for (int i = 0; i < num; ++i)
	{
		int size = 15;
#if debug_sec
		size = 4;
#endif
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
		std::cout << " heapify({";
		for (int i = 0; i < n; ++i)
		{
			std::cout << notHeap[i] << (i == n-1 ? "})" + eq + "{" : ",");
		}
		for (int i = 0; i < n; ++i)
		{
			std::cout << heap[i] << (i == n-1 ? "}" : ",");
		}
		std::cout << std::endl;
	}
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
			ins[++c] = p;
		}
	}
	return ins;
}

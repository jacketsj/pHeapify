#ifndef H_pHeapify
#define H_pHeapify
#include <utility>
#include <climits>
#include "omp.h"
#define debug_sec true

//temp for writing: biggy for int256_t, mini for int8_t
//since these sizes are not natively supported by C++ or by OpenMP, ints can be used instead with caution
//note that this substitution will only work for arrays smaller than 32 in size
#define mini int
#define biggy int
#define lowest INT_MIN

mini* pHeapify(mini* A, mini n);
biggy exp(mini am, mini km);
std::pair<mini,mini> max_loc(mini* A, mini lo, mini hi, biggy combo, biggy* powLookup);
mini count(mini* A, mini lo, mini hi, biggy combo, biggy* powLookup);
void completeHeap(mini* heap, mini n, mini* arr_max, biggy* arr_left, biggy* arr_right, mini i, biggy combo, int* A);

#endif

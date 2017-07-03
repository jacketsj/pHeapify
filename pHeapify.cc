#include "pHeapify.h"
#include "omp.h"
#include <climits>
#include <utility>

#if debug_sec
#include <iostream>
#endif

mini* pHeapify(mini* A, mini n)
{
	if (n <= 1)
	{
		return A;
	}

	//number of combinations
	biggy comboCount = exp(2, n);
	
	//calculate powers
	biggy* powLookup = new biggy[n];
	omp_set_num_threads(n);
	#pragma omp parallel
	{
		mini id = omp_get_thread_num();
		powLookup[id] = exp(2, id);
	}
	
	//find maxes

	//stores indeces
	mini* arr_max = new mini[comboCount];
	
	omp_set_num_threads(comboCount);

	#pragma omp parallel
	{
		biggy id = omp_get_thread_num();
#if debug_sec
		#pragma omp single
		{
			std::cout << id;
		}
#endif
		arr_max[id] = max_loc(A, 0, n-1, id, powLookup).second;
	}

#if debug_sec
	std::cout << "arr_max = {";
	for (int i = 0; i < comboCount; ++i)
	{
		std::cout << arr_max[i] << (i == comboCount - 1 ? "}" : ",");
	}
	std::cout << std::endl;
	std::cout << "powLookup = {";
	for (int i = 0; i < n; ++i)
	{
		std::cout << i << ":" << powLookup[i] << (i == n - 1 ? "}" : ",");
	}
	std::cout << std::endl;
#endif

	//find selected indeces

	//stores pointers to arrays of indeces for combos
	//mini** arr_selec = new mini*[comboCount];
	mini* arr_count = new mini[comboCount];
	biggy* arr_left = new biggy[comboCount];
	biggy* arr_right = new biggy[comboCount];
	
	omp_set_num_threads(comboCount);
	
	#pragma omp parallel
	{
		biggy id = omp_get_thread_num();
		arr_count[id] = count(A, 0, n-1, id, powLookup);
		mini roundedDown = arr_count[id]; 

		//round down to nearest power of two sub 1
		//binary search
		mini lo = 0, hi = id;
		while (hi > lo + 1)
		{
			mini half = (hi - lo) / 2;
			if (powLookup[half] - 1 > roundedDown)
			{
				hi = half;
			}
			else
			{
				lo = half;
			}
		}
		roundedDown = lo;

		mini rem = arr_count[id] - lo;
		mini minrem = rem;
		if (minrem > (roundedDown + 1))
		{
			minrem = (roundedDown + 1);
		}
		//last position in left subheap (or first in next with zero indeces)
		mini mid = roundedDown + minrem;
		
		biggy bigMaxPos = powLookup[arr_max[id]];
		biggy leftMatches = powLookup[mid+1]-1;
		arr_left[id] = id & leftMatches & (~bigMaxPos);
		arr_right[id] = id & (~leftMatches) & (~bigMaxPos);
		
		//arr_selec[id] = selected(A, 0, n-1, id, powLookup).second;
	}

	//now do parallel recursion step to find final heap
	mini* heap = new mini[n];
	completeHeap(heap, n, arr_max, arr_left, arr_right, 0, comboCount-1);
	return heap;
}

//returns a^k
biggy exp(mini am, mini km)
{
	biggy a = biggy(am), k = biggy(km);
	biggy ret = 1;
	biggy dub = 2;
	while (k > 0)
	{
		if (k % 2 == 1)
		{
			ret *= dub;
		}
		k /= 2;
		dub *= dub;
	}
	return ret;
}

std::pair<mini,mini> max_loc(mini* A, mini lo, mini hi, biggy combo, biggy* powLookup)
{
	//base cases
	//if no more numbers, or this num is not included in combo
	if (lo > hi || (lo == hi && (combo & powLookup[lo] != powLookup[lo])))
	{
		//return index of later non-number for priority
		std::pair<mini,mini> ret(lowest, lo);
		return ret;
	}
	//if this num is included and alone
	else if (lo == hi)
	{
		std::pair<mini,mini> ret(A[lo], lo);
		return ret;
	}
	//here would be a good place for a sequential cutoff

	//first=val, second=loc
	std::pair<mini,mini> left, right;
	#pragma omp task untied shared(A, lo, hi, combo, left)
	{
		left = max_loc(A, lo, (hi + lo) / 2, combo, powLookup);
	}
	right = max_loc(A, (hi + lo) / 2 + 1, hi, combo, powLookup);
	#pragma omp taskwait
	if (left.first > right.first || (left.first == right.first && left.second < right.second))
	{
		return left;
	}
	else
	{
		return right;
	}
}

mini count(mini* A, mini lo, mini hi, biggy combo, biggy* powLookup)
{
	//base cases
	//if no more numbers, or this num is not included in combo
	if (lo > hi || (lo == hi && (combo & powLookup[lo] != powLookup[lo])))
	{
		return 0; //return index of later non-number for priority
	}
	//if this num is included and alone
	else if (lo == hi)
	{
		return 1;
	}
	//here would be a good place for a sequential cutoff

	//first=val, second=loc
	mini left, right;
	#pragma omp task untied shared(A, lo, hi, combo, left)
	{
		left = count(A, lo, (hi + lo) / 2, combo, powLookup);
	}
	right = count(A, (hi + lo) / 2 + 1, hi, combo, powLookup);
	#pragma omp taskwait
	return left + right;
}

void completeHeap(mini* heap, mini n, mini* arr_max, biggy* arr_left,
			biggy* arr_right, mini i, biggy combo)
{
	if (i > n || combo == 0)
	{
#if debug_sec
		std::cout << "here " << combo << std::endl;
#endif
		return;
	}
#if debug_sec
	std::cout << "heap[" << i << "]=" << arr_max[combo] << ";" << std::endl;
#endif
	heap[i] = arr_max[combo];
	#pragma omp task untied shared(heap, n, arr_max, arr_left, arr_right, i, combo)
	{
		completeHeap(heap, n, arr_max, arr_left, arr_right, 2*i+1, arr_left[combo]);
	}
	completeHeap(heap, n, arr_max, arr_left, arr_right, 2*i+2, arr_right[combo]);
	#pragma omp taskwait //only serves to make parent (non-recursive) function to wait
}

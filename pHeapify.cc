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

#if debug_sec
	int* present = new int[omp_get_max_threads()];
	omp_set_dynamic(false);
	#pragma omp parallel
	{
		present[omp_get_thread_num()] = 1;
	}
	int total = 0;
	for (int i = 0; i < omp_get_max_threads(); ++i)
	{
		total += present[i];
	}
	std::cout << "TOTAL = " << total << std::endl;
#endif

	//dynamic teams not supported
	omp_set_dynamic(false);

	omp_set_max_active_levels(5);

	//number of combinations
	biggy comboCount = exp(2, n);
	
	//calculate powers
	biggy* powLookup = new biggy[n];
	omp_set_num_threads(n);
#if debug_sec
	int* powIds = new int[n];
	int numIds = 0;
#endif
	#pragma omp parallel
	{
		mini id = omp_get_thread_num();
		powLookup[id] = exp(2, id);
#if debug_sec
		powIds[id] = id;
		++numIds;
//		#pragma omp single
//		{
			std::cout << omp_get_num_threads() << " out of " << omp_get_max_threads() << std::endl;
//		}
#endif
	}
	
	//find maxes

	//stores indeces
	mini* arr_max = new mini[comboCount];
	
	omp_set_num_threads(comboCount);

	#pragma omp parallel
	{
		biggy id = omp_get_thread_num();
		arr_max[id] = max_loc(A, 0, n-1, id, powLookup).second;
#if debug_sec
		#pragma omp single
		{
			std::cout << id;
		}
#endif
	}

#if debug_sec
	std::cout << numIds << std::endl;

	std::cout << "arr_max = {";
	for (int i = 0; i < comboCount; ++i)
	{
		std::cout << arr_max[i] << (i == comboCount - 1 ? "}" : ",");
	}
	std::cout << std::endl;
	std::cout << "powLookup = {";
	for (int i = 0; i < n; ++i)
	{
		std::cout << powIds[i] << ":" << powLookup[i] << (i == n - 1 ? "}" : ",");
	}
	std::cout << std::endl;
#endif

	//find selected indeces

	//stores pointers to arrays of indeces for combos
	//mini** arr_selec = new mini*[comboCount];
	mini* arr_count = new mini[comboCount];
	biggy* arr_left = new biggy[comboCount];
	biggy* arr_right = new biggy[comboCount];
	
#if debug_sec
	int* roundedDowns = new int[comboCount];
#endif
	int* mids = new int[comboCount];
	omp_set_num_threads(comboCount);
	

	#pragma omp parallel
	{
		biggy id = omp_get_thread_num();
		arr_count[id] = count(0, n-1, id, powLookup);
		mini roundedDown = (arr_count[id]-1)/2; 
		
  
		//TODO: Bug flag. Something is (probably) wrong with this code. Slower code available for debugging.
		//round down to nearest power of two sub 1
		//binary search
		mini lo = 0, hi = arr_count[id];
		while (hi > lo + 1
			&& !((roundedDown & (powLookup[lo] - 1)) != roundedDown
				&& (roundedDown & (powLookup[lo+1] - 1)) == roundedDown))
		{
#if debug_sec
			//temporary linear loop for debugging
			++lo;
#else
			mini half = (hi + lo) / 2;
			if ((roundedDown & (powLookup[half] - 1)) != roundedDown)
			{
				hi = half;
			}
			else
			{
				lo = half;
			}
#endif
		}
		//if we really should round down in the first place (not already a power of 2 sub 1)
		if (roundedDown != powLookup[lo+1] - 1)
		{
			roundedDown = powLookup[lo] - 1;
		}
		mini rem = arr_count[id] - lo - 1; //-1 for no more root
		mini minrem = rem;
		if (minrem > (roundedDown + 1))
		{
			minrem = (roundedDown + 1);
		}
		//number of values on left subheap
		mids[id] = roundedDown + minrem;
#if debug_sec
		roundedDowns[id] = roundedDown;
#endif
	}
#if debug_sec
	std::cout << "roundedDowns = {";
	for (int i = 0; i < comboCount; ++i)
	{
		std::cout << roundedDowns[i] << (i == comboCount - 1 ? "}" : ",");
	}
#endif
	std::cout << std::endl;
	#pragma omp parallel
	{
		biggy id = omp_get_thread_num();
		//last position in left subheap (or first in right with zero indeces)
		//calculated by binary search on 
		mini countToMid = mids[id];
		mini lo = n-countToMid-1, hi = n;
		if (lo < 0)
		{
			lo = 0;
		}
#if debug_sec
		int testCount = 0;
		if (arr_count[((powLookup[lo+1]-1) & id)] < countToMid)
		{
			std::cout << "c2mid=" << countToMid << ",n-1=" << n-1 << ",powLookup[lo+1]=" << powLookup[lo+1]
				<< ",arr_count[stuff]=" << arr_count[((powLookup[lo+1]-1) & id)] << ",id=" << id << std::endl; 
		}
#endif
		//TODO: Bug flag. Something is wrong with this code.
#if debug_sec
		//slower test code
		//offensive programming practice revealed that the orinal generalized test always failed
		while (arr_count[((powLookup[lo+1]-1) & id)] < (n-countToMid-1))
		{
			++lo;
			--hi; //just fixing Wall for debug
			++testCount;
		}
		mini midloc = powLookup[lo]-1;
#endif
		/*
		while (hi > lo + 1 && arr_count[((powLookup[lo+1]-1) & id)] < countToMid)
		{
#if debug_sec
			//std::cout << lo << std::endl;
			//std::cout << "c2mid=" << countToMid << ",n-1=" << n-1 << ",powLookup[lo+1]=" << powLookup[lo+1]
			//	<< ",arr_count[stuff]=" << arr_count[((powLookup[lo+1]-1) & id)] << ",id=" << id << std::endl; 
			//break;
#endif
			mini half = (hi + lo) / 2;
			if (arr_count[((powLookup[half+1]-1) & id)] >= countToMid)
			{
				hi = half;
#if debug_sec
				++testCount;
#endif
			}
			else
			{
				lo = half;
			}
		}
		mini midloc = lo;
		if (arr_count[((powLookup[lo+1]-1) & id)] < countToMid)
		{
#if debug_sec
			std::cout << "it did the thing: " << testCount << "," << countToMid << hi << lo << std::endl;
#endif
			midloc = hi;
		}
		*/
		
		biggy bigMaxPos = powLookup[arr_max[id]];
		biggy leftMatches = midloc;
		arr_left[id] = id & leftMatches & (~bigMaxPos);
		arr_right[id] = id & (~leftMatches) & (~bigMaxPos);
		
		//arr_selec[id] = selected(A, 0, n-1, id, powLookup).second;
	}

#if debug_sec
	std::cout << "arr_left = {";
	for (int i = 0; i < comboCount; ++i)
	{
		std::cout << arr_left[i] << (i == comboCount - 1 ? "}" : ",");
	}
	std::cout << std::endl;
	std::cout << "arr_right = {";
	for (int i = 0; i < comboCount; ++i)
	{
		std::cout << arr_right[i] << (i == comboCount - 1 ? "}" : ",");
	}
	std::cout << std::endl;
	std::cout << "arr_count = {";
	for (int i = 0; i < comboCount; ++i)
	{
		std::cout << arr_count[i] << (i == comboCount - 1 ? "}" : ",");
	}
	std::cout << std::endl;
	std::cout << "mids = {";
	for (int i = 0; i < comboCount; ++i)
	{
		std::cout << mids[i] << (i == comboCount - 1 ? "}" : ",");
	}
	std::cout << std::endl;
#endif

	//now do parallel recursion step to find final heap
	mini* heap = new mini[n];
	completeHeap(heap, n, arr_max, arr_left, arr_right, 0, comboCount-1, A);
	return heap;
}

//returns a^k
biggy exp(mini am, mini km)
{
	biggy dub = biggy(am), k = biggy(km);
	biggy ret = 1;
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
	if (lo > hi || (lo == hi && ((combo & powLookup[lo]) != powLookup[lo])))
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
#if debug_sec
		//std::cout << "left.first beats right.first: (" << left.first << "," << left.second << ") > (" << right.first << "," << right.second << ")" << std::endl;
#endif
		return left;
	}
	else
	{
#if debug_sec
		//std::cout << "right.first beats left.first: (" << left.first << "," << left.second << ") < (" << right.first << "," << right.second << ")" << std::endl;
#endif
		return right;
	}
}

//bool determines if this returns the id of the legal values or the count of them
mini count(mini lo, mini hi, biggy combo, biggy* powLookup)
{
	//base cases
	//if no more numbers, or this num is not included in combo
	if (lo > hi || (lo == hi && ((combo & powLookup[lo]) != powLookup[lo])))
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
	#pragma omp task untied shared(lo, hi, combo, left)
	{
		left = count(lo, (hi + lo) / 2, combo, powLookup);
	}
	right = count((hi + lo) / 2 + 1, hi, combo, powLookup);
	#pragma omp taskwait
	return left + right;
}

void completeHeap(mini* heap, mini n, mini* arr_max, biggy* arr_left,
			biggy* arr_right, mini i, biggy combo, int* A)
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
	heap[i] = A[arr_max[combo]];
	#pragma omp task untied shared(heap, n, arr_max, arr_left, arr_right, i, combo)
	{
		completeHeap(heap, n, arr_max, arr_left, arr_right, 2*i+1, arr_left[combo], A);
	}
	completeHeap(heap, n, arr_max, arr_left, arr_right, 2*i+2, arr_right[combo], A);
	#pragma omp taskwait //only serves to make parent (non-recursive) function to wait
}

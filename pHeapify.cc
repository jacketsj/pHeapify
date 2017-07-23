#include "pHeapify.h"
#include "omp.h"
#include <climits>
#include <utility>

mini* pHeapify(mini* A, mini n)
{
	if (n <= 1)
	{
		//need to be consistent with pointer counts in base case
		mini* A0 = new mini[n];
		if (n == 1)
		{
			A0[0] = A[0];
		}
		return A0;
	}

	//dynamic teams not supported
	omp_set_dynamic(false);

	//maximize parallelization
	omp_set_max_active_levels(5);

	//number of combinations
	biggy comboCount = exp(2, n);
	
	//calculate powers
	biggy* powLookup = new biggy[n];
	omp_set_num_threads(n);

	//calculate powers of two
	#pragma omp parallel
	{
		mini id = omp_get_thread_num();
		powLookup[id] = exp(2, id);
	}
	
	//find maxes:

	//stores indeces
	mini* arr_max = new mini[comboCount];
	
	omp_set_num_threads(comboCount);

	#pragma omp parallel
	{
		biggy id = omp_get_thread_num();
		arr_max[id] = max_loc(A, 0, n-1, id, powLookup).second;
	}

	//find selected indeces

	//stores pointers to arrays of indeces for combos
	//mini** arr_selec = new mini*[comboCount];
	mini* arr_count = new mini[comboCount];
	biggy* arr_left = new biggy[comboCount];
	biggy* arr_right = new biggy[comboCount];
	
	mini* mids = new mini[comboCount];
	omp_set_num_threads(comboCount);
	

	#pragma omp parallel
	{
		biggy id = omp_get_thread_num();
		arr_count[id] = count(0, n-1, id, powLookup);
		mini roundedDown = (arr_count[id]-1)/2; 
		
  
		//TODO: Bug flag. Something is (probably) wrong with this code. Slower code available for debugging.
		//round down to nearest power of two sub 1 (binary search finds the round up, then check for equality case)
		//binary search

		//choice of hi will have to be changed if size of mini and biggy is changed
		mini lo = 0, hi = n; //hi should be number of bits or just n, due to lack of table
		while (!(powLookup[lo+1] - 1 >= roundedDown && powLookup[lo] - 1 < roundedDown) && roundedDown > 0)
		{
			if (hi == lo + 1)
			{
				hi = lo;
				//equivalently, use break
				//continue used for offensive testing
				continue;
			}
			mini half = (hi + lo) / 2;
			if (powLookup[half] - 1 >= roundedDown)
			{
				hi = half;
			}
			else
			{
				lo = half;
			}
		}
		//if we need to round down...
		if (powLookup[lo+1] - 1 > roundedDown)
		{
			//then do so
			roundedDown = powLookup[lo] - 1;
		}

		mini rem = arr_count[id] - 2*roundedDown - 1; //-1 for no more root
		if (rem == -1)
		{
			rem = 0;
		}
		mini minrem = rem;
		//if the remainder surpasses the halfway point
		if (minrem > ((arr_count[id] - rem) % 2) + (arr_count[id] - rem) / 2)
		//if (minrem > (roundedDown + 1))
		{
			//make it the count before the halfway point
			//minrem = (roundedDown + 1);
			minrem = ((arr_count[id] - rem) % 2) + (arr_count[id] - rem) / 2;
		}
		//number of values on left subheap
		mids[id] = roundedDown + minrem;
	}

	#pragma omp parallel
	{
		biggy id = omp_get_thread_num();
		biggy bigMaxPos = powLookup[arr_max[id]];
		//last position in left subheap (or first in right with zero indeces)
		//calculated by binary search on 
		mini countToMid = mids[id];
		//making lo n-countToMid-1 vs n-countToMid changes which tests pass
		mini lo = 0, hi = n;
		//mini lo = n-countToMid-1, hi = n;
		if (lo < 0)
		{
			lo = 0;
		}

		while (arr_count[((powLookup[lo]-1) & id & (~bigMaxPos))] < countToMid)
		{
			mini half = (hi + lo) / 2;
			if (lo + 1 >= hi)
			{
				lo = hi = lo + 1;
				break;
			}
			if (arr_count[((powLookup[half]-1) & id & (~bigMaxPos))] >= countToMid)
			{
				hi = half;
			}
			else
			{
				lo = half;
			}
		}

		biggy midloc = powLookup[lo]-1;
		
		biggy leftMatches = midloc;
		arr_left[id] = id & leftMatches & (~bigMaxPos);
		arr_right[id] = id & (~leftMatches) & (~bigMaxPos);
	}

	//now do parallel recursion step to find final heap
	mini* heap = new mini[n];
	completeHeap(heap, n, arr_max, arr_left, arr_right, 0, comboCount-1, A);

	//clean temp memory
	delete[] powLookup;
	delete[] arr_max;
	delete[] arr_count;
	delete[] arr_left;
	delete[] arr_right;
	delete[] mids;

	return heap;
}

//returns a^k
biggy exp(mini am, mini km)
{
	biggy *dub = new biggy(am), *k = new biggy(km);
	biggy ret = 1;
	while (*k > 0)
	{
		if (*k % 2 == 1)
		{
			ret *= *dub;
		}
		*k /= 2;
		*dub *= *dub;
	}
	delete dub;
	delete k;
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
		return left;
	}
	else
	{
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
			biggy* arr_right, mini i, biggy combo, mini* A)
{
	if (i > n || combo == 0)
	{
		return;
	}
	heap[i] = A[arr_max[combo]];
	#pragma omp task untied shared(heap, n, arr_max, arr_left, arr_right, i, combo)
	{
		completeHeap(heap, n, arr_max, arr_left, arr_right, 2*i+1, arr_left[combo], A);
	}
	completeHeap(heap, n, arr_max, arr_left, arr_right, 2*i+2, arr_right[combo], A);
	#pragma omp taskwait //only serves to make parent (non-recursive) function to wait
}

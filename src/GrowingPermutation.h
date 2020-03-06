#pragma once
#include <vector>
#include <list>


template<  int PermutationSize,int OverlapSize>
class GrowingPermutation :private std::list<int>
{
	using Indexes = std::array<int, PermutationSize>;
	bool isNotAppearing(int col) {
		for (auto e : (*this)) {
			if (col == e)
				return false;
		}
		return true;
	}
	bool tryAddLeft(const Indexes & indexes ,int overlapSize) {

		const_iterator it = begin();
		int ix = PermutationSize - overlapSize;
		for (; ix < PermutationSize && indexes[ix] == *it; ix++, it++);
		if (ix != PermutationSize)
			return false;
		for (ix = 0; ix < PermutationSize - overlapSize&& isNotAppearing(indexes[ix]); ix++);
		if (ix != (PermutationSize - overlapSize))
			return false;

		for (ix = 0; ix < (PermutationSize - overlapSize); ix++)
			push_front(indexes[ix]);
		return true;
	}


	bool tryAddRight(const Indexes & indexes, int overlapSize) {
		const_reverse_iterator it = rbegin();;

		std::advance(it, overlapSize-1);
		int ix = 0;
		for (; ix < overlapSize&&indexes[ix] == *it; ix++, it--);
		if (ix != overlapSize)
			return false;

		for (; ix < PermutationSize && isNotAppearing(indexes[ix]); ix++);
		if (ix != PermutationSize)
			return false;

		for (ix = overlapSize; ix != PermutationSize; ix++)
			push_back(indexes[ix]);
		return true;
	}


	bool tryAddLeft(const Indexes & indexes) {

		const_iterator it = begin();
		int ix = PermutationSize - OverlapSize;
		for (; ix < PermutationSize && indexes[ix] == *it; ix++, it++);
		if (ix != PermutationSize)
			return false;
		for (ix = 0; ix < PermutationSize - OverlapSize&& isNotAppearing(indexes[ix]); ix++);
		if (ix != (PermutationSize - OverlapSize))
			return false;

		for (ix = 0; ix < (PermutationSize - OverlapSize); ix++)
			push_front(indexes[ix]);
		return true;
	}


	bool tryAddRight(const Indexes & indexes) {
		const_reverse_iterator it = rbegin();;

		std::advance(it, OverlapSize-1);
		int ix = 0;
		for (; ix < OverlapSize&&indexes[ix] == *it; ix++, it--);
		if (ix != OverlapSize)
			return false;

		for (; ix < PermutationSize && isNotAppearing(indexes[ix]); ix++);
		if (ix != PermutationSize)
			return false;

		for (ix = OverlapSize; ix != PermutationSize; ix++)
			push_back(indexes[ix]);
		return true;
	}
public:
	GrowingPermutation(){}
	GrowingPermutation(Indexes& indexes) {

		init(indexes);
	}

	void init(const Indexes & indexes){
		clear();
		for (auto e : indexes)
			push_back(e);
	}

	bool tryAdd(const Indexes & indexes) {
		if (!tryAddRight(indexes)&&!tryAddLeft(indexes)  )
			return false;
		return true;
	}

	bool tryAdd(const Indexes & indexes,int overlapSize) {
		if (!tryAddRight(indexes,overlapSize) && !tryAddLeft(indexes,overlapSize))
			return false;
		return true;
	}
	size_t size()const { return std::list<int>::size(); }
	std::vector<int> getVecCols()const {
		std::vector<int> vec(begin(), end());
	return vec;
	}
	void clear() { std::list<int>::clear(); }
};

#pragma once
#include <vector>
#include <list>


template<  int PermutationSize, int OverlapSize, typename ElementType_>
class GrowingPermutation :private std::list<ElementType_>
{
	using ElementType = ElementType_;
	using Indexes = std::array<ElementType, PermutationSize>;
	bool isNotAppearing(int col) const{
		for (auto e : (*this)) {
			if (col == e)
				return false;
		}
		return true;
	}
public:
	bool tryAddLeft(const Indexes & indexes, int overlapSize) {
		if (!canBeAddedLeft(indexes, overlapSize))
			return  false;
		addToLeft(indexes);
		return true;
	}

	bool canBeAddedLeft(const Indexes & indexes, int overlapSize) {
		const_iterator it = begin();
		int ix = PermutationSize - overlapSize;
		for (; ix < PermutationSize && indexes[ix] == *it; ix++, it++);
		if  (ix != PermutationSize)
			return false;
		for (ix=0; ix < PermutationSize - overlapSize&& isNotAppearing(indexes[ix]); ix++);
		return (ix == (PermutationSize - overlapSize));
	}


	bool canBeAddedToRight(const Indexes& indexes, int overlapSize) const {
		const_iterator it = end();
		std::advance(it,- overlapSize );
		int ix = 0;
		for (; ix < overlapSize&&indexes[ix] == *it; ix++, it++);
		if (ix != overlapSize)
			return false;
		for (; ix < PermutationSize && isNotAppearing(indexes[ix]); ix++);
		return  (ix == PermutationSize);
	}

	bool tryAddRight(const Indexes & indexes, int overlapSize) {
		if (!canBeAddedToRight(indexes,overlapSize))
			return false;
		addToRight(indexes);
		return true;
	}


	bool canBeAddedLeft(const Indexes & indexes) {
		const_iterator it = begin();
		int ix = PermutationSize - OverlapSize;
		for (; ix < PermutationSize && indexes[ix] == *it; ix++, it++);
		if (ix != PermutationSize)
			return false;
		for (ix=0; ix < PermutationSize - OverlapSize&& isNotAppearing(indexes[ix]); ix++);
		return (ix == (PermutationSize - OverlapSize));
	}

	bool tryAddLeft(const Indexes & indexes) {
		if (!canBeAddedLeft(indexes))
			return false;
		addToLeft(indexes);
		return true;
	}

	bool addToLeft(const Indexes & indexes) {
		int ix = 0;
		for (; ix < (PermutationSize - OverlapSize); ix++)
			push_front(indexes[ix]);
		return true;
	}

	bool tryAddRight(const Indexes & indexes) {
		if (!canBeAddedRight(indexes))
			return false;
		addToRight(indexes);
		return true;
	}

	bool canBeAddedRight(const Indexes & indexes) {
		const_iterator it = end();;
		std::advance(it,- OverlapSize );
		int ix = 0;
		for (; ix < OverlapSize&&indexes[ix] == *it; ix++, it++);
		if (ix != OverlapSize)
			return false;
		for (; ix < PermutationSize && isNotAppearing(indexes[ix]); ix++);
		return  (ix == PermutationSize);
	}

	bool addToRight(const Indexes & indexes) {
		int ix = OverlapSize;
		for (; ix != PermutationSize; ix++)
			push_back(indexes[ix]);
		return true;
	}

	GrowingPermutation() {}

	GrowingPermutation(Indexes& indexes) {

		init(indexes);
	}

	void init(const Indexes & indexes) {
		clear();
		for (auto e : indexes)
			push_back(e);
	}

	bool tryAdd(const Indexes & indexes) {
		if (!tryAddRight(indexes) && !tryAddLeft(indexes))
			return false;
		return true;
	}

	bool tryAdd(const Indexes & indexes, int overlapSize) {
		if (!tryAddRight(indexes, overlapSize) && !tryAddLeft(indexes, overlapSize))
			return false;
		return true;
	}
	size_t size()const { return std::list<ElementType>::size(); }
	std::vector<ElementType> getVecCols()const {
		std::vector<ElementType> vec(begin(), end());
		return vec;
	}
	void clear() { std::list<ElementType>::clear(); }
};



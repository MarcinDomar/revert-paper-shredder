#pragma once
#include "types.h"
using Index2Perm = std::array<int, 2>;

class InitializerOfIndexs2Permutaion {
	const int size_of_set;
public:
	Index2Perm getFirst()const { return{ 0,1 }; }
	InitializerOfIndexs2Permutaion(int size_of_set) :size_of_set(size_of_set) {}
	bool initToNext(Index2Perm & index)const {
		index[1]++;
		if (index[1] == index[0])
		{
			index[1]++;
			if (index[1] == size_of_set)
			{
				index[0] = 0;
				index[1] = 1;
				return false;
			}
		}
		else if  (index[1] == size_of_set )
		{
			index[0]++;
			index[1] = 0;
		}
		return true;
	}
	Index2Perm  getNs(int dist) const{
		int first = dist / (size_of_set - 1);
		int second=dist % (size_of_set - 1);

		return{ first,(first > second) ? second : second + 1 };
	}
	int howManyPermutaion()const { return (size_of_set )*(size_of_set-1); }
};

template <int N,typename ColIndexType_=unsigned char>
class InitializerOfIndexsNPermutation {
	std::array< unsigned int , N - 1> ranges;
	std::vector<unsigned int > const_numbers_of_indexes ;
public :
	using ColIndexType = ColIndexType_;
	using Indexes = std::array<ColIndexType, N>;
	const int size_of_set;
	bool initNextI(int ix, Indexes & index) const {
		index[ix]++;
		incrIfNeeded(ix, index);
		return index[ix] != size_of_set;
	}
	void incrIfNeeded(int ix, Indexes & index) const
	{
		int j = 0;
		while (j != ix) {
			for (; j < ix && index[ix] != index[j]; j++);
			if (j != ix) {
				index[ix]++;
				j = 0;
			}
		}
	}
public:
	Indexes getFirst()const {
		Indexes indexes;
		for (int i = 0; i < N; i++)indexes[i] = i;
		return indexes;
	}

	InitializerOfIndexsNPermutation(int size_of_set) :size_of_set(size_of_set) ,const_numbers_of_indexes(size_of_set){
		ranges[N - 2] = size_of_set - N + 1;
		for (int i = N - 3; i >= 0; i--)
			ranges[i] = ranges[i + 1] * (size_of_set - i - 1);
		for (int i = 0; i < size_of_set; i++) const_numbers_of_indexes[i] = i;
	}
	bool initToNext(Indexes & index)const {
		int i = N-1;
		for (; i >= 0; i--) {
			if (initNextI(i, index))
				break;
		}
		if (i == -1) {
			index = getFirst();
			return false;
		}
		int next = 0;
		for (i++; i < N; i++) {
			index[i] = next;
			incrIfNeeded(i, index);
			next = index[i] + 1;
		}
		return true;
	}
	Indexes  getIndexes(int numberOfPermutation) const {
		Indexes indexes;
		std::vector<int> numbers(const_numbers_of_indexes.begin(), const_numbers_of_indexes.end());

		for (size_t i = 0; i < N-1; i++) {
			indexes[i] = numberOfPermutation / ranges[i];
			numberOfPermutation %= ranges[i];
		}
		indexes[N - 1] = numberOfPermutation;
		for (int i=0;i<N;i++)
		{
			auto ix = indexes[i];
			auto val = numbers[ix];
			indexes[i] = val;
			numbers.erase(numbers.begin() + ix);
		}
		return indexes;
	}
	unsigned int getSizeOfAllPermutations()const { return (size_of_set)*ranges[0]; }
};

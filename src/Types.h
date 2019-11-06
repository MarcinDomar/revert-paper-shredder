#pragma once
#include <vector>
#include <string>
#include <array>
#include <unordered_map>

using ListOfRows = std::list < std::list<std::string>>;
using TwoChars = std::array<char, 2>;
using ThreeChars = std::array<char, 3>;
using FourChars = std::array<char, 4>;
using VectorOrRows = std::vector<std::vector<TwoChars>>;
using PaperSide = std::vector<std::string>;
using ColumnsPermutation = std::vector<int>;
using Index2Perm = std::array<int, 2>;
using Index3Perm = std::array<int, 3>;


class InitializerOfIndex2Permutaion {
	const int size_of_set;
public:
	Index2Perm getFirst()const { return{ 0,1 }; }
	InitializerOfIndex2Permutaion(int size_of_set) :size_of_set(size_of_set) {}
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
	unsigned int howManyPermutaion()const { return (size_of_set )*(size_of_set-1); }
};

class InitializerOfIndex3Permutaion {
	const int size_of_set;
	bool initNextI(int ix, Index3Perm & index) const  {
		index[ix]++;
		incrIfNeeded(ix,index);
		return index[ix] != size_of_set;
	}
	void incrIfNeeded(int ix, Index3Perm & index) const
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
	Index3Perm getFirst()const { return{ 0,1,2 }; }
	InitializerOfIndex3Permutaion(int size_of_set) :size_of_set(size_of_set) {}
	bool initToNext(Index3Perm & index)const {
		int i = 2;
		for (; i >=0; i--){
			if (initNextI(i, index))
				break;
		}
		if (i == -1) {
			index = getFirst();
			return false;
		}
		int next = 0;
		for (i++; i < 3; i++) {
			index[i] = next;
			incrIfNeeded(i, index);
			next = index[i] + 1;
		}
	}
	Index3Perm  getNs(int dist) const {
		int first = dist / ((size_of_set - 1)*(size_of_set-2));
		int second = (dist % ((size_of_set - 1)*(size_of_set - 2)))/(size_of_set-2);
		int third = (dist % ((size_of_set - 1)*(size_of_set - 2))) % (size_of_set - 2);
		second = (first > second) ? second : second + 1;
		third = (second > third) ? third : third + 1;
		third = (first > third) ? third : third + 1;
		return{ first,second ,third};
	}
	unsigned int howManyPermutaion()const { return (size_of_set)*(size_of_set - 1)*(size_of_set-2); }
};


#pragma once
#include <vector>
#include <string>
#include <array>
#include <unordered_map>

using ListOfRows = std::list < std::list<std::string>>;
using TwoChars = std::array<char, 2>;
using VectorOrRows = std::vector<std::vector<TwoChars>>;
using PaperSide = std::vector<std::string>;

using Index2Perm = std::array<int, 2>;
class InitializerOfIndex2Permutaion {
	const int size_of_set;
public:
	Index2Perm getFirst()const { return{ 0,1 }; }
	InitializerOfIndex2Permutaion(int size_of_set) :size_of_set(size_of_set) {}
	bool initToNext(Index2Perm & index)const {
		if (index[0] == size_of_set - 1)
			return false;
		index[1]++;
		if (index[1] == size_of_set) {
			index[0]++;
			index[1] = 0;
		}
	}
	Index2Perm  getNs(int dist) const{
		int first = dist / (size_of_set - 1);
		int second=dist % (size_of_set - 1);

		return{ first,(first > second) ? second : second + 1 };
	}
	unsigned int howManyPermutaion()const { return (size_of_set - 1)*(size_of_set-1); }
};



#include "NarrowerSearches.h"
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <future>
#include <thread>



void NarrowerSearches::buildScores() {
	
	auto worker = [this](int dist_from_frist, int size) {
		std::array<char,4>  tw;
		Index2Perm index = indexNexter.getNs(dist_from_frist);
		for (int i = 0; i < size; i++) {
			auto & score = cols2_scores[i + dist_from_frist];
			for (int j = 0; j < inputData.size(); j++)
			{
				auto &left = inputData[j][ index[0]];
				auto &right = inputData[j][index[1]];
				std::copy(left.begin(), left.end(), tw.begin());
				std::copy(right.begin(), right.end(), tw.begin()+2);
				score += rater.getScore(tw);
			}
			indexNexter.initToNext(index);
		}
		return 0;
	};
	using Future = std::future<int>;
	std::list<Future> futures;
	
	int size = indexNexter.howManyPermutaion() / std::thread::hardware_concurrency();
	int i = 0;
	for (i; i < std::thread::hardware_concurrency()-1; i++)
	{
		futures.push_back(std::async(std::launch::async, worker, i*size, size));

	}
	futures.back() = std::async(std::launch::async, worker, i*size,indexNexter.howManyPermutaion()  - size*i );

	while (futures.size()) {
		futures.front().get();
		futures.pop_front();
	}

}

std::vector<PaperSide> NarrowerSearches::getBestSugestions(int numbersOfSugestins) {
	using TempPermutation = std::list<int>;
	std::vector<CoumnsPermutation>  permutations;
	unsigned int start = 0;
	int i = 0;
	for (; i < numbersOfSugestins; i++) {
		int next = start;

		TempPermutation  permutation ;
		auto isNotAddedEarlier = [&permutation](int col) {
			for (auto el_col : permutation) {
				if (el_col == col)
					return false;
			}
			return true;
		};

		permutation.push_back(indexes[next][0]);
		permutation.push_back( indexes[next][1]);
		next++;
		while (next != indexes.size()&&permutation.size()<cols) {
			if (permutation.back() == indexes[next][0] && isNotAddedEarlier(indexes[next][1])) {
				permutation.push_back(indexes[next][1]);
				next++;
			} else 	if (permutation.front() == indexes[next][1] && isNotAddedEarlier(indexes[next][0])) {
					permutation.push_front(indexes[next][0]);
					next++;
			}
			next++;
		}
		if (permutation.size() == cols) {
			permutations.push_back(CoumnsPermutation(permutation.begin(), permutation.end()));
			start++;
		}
		else
			break;
	}
	return getPapers(permutations);
}

std::vector<PaperSide> NarrowerSearches::getPapers(const std::vector<CoumnsPermutation> & colesPermutatios)const
{
	std::vector<PaperSide> results;
	for (int i = 0; i < colesPermutatios.size(); i++) {
		PaperSide paperSide;
		for (int row = 0; row != rows; row++) {
			std::string str;
			for (auto ixCol : colesPermutatios[i])
				str.insert(str.end(), inputData[row][ixCol].begin(), inputData[row][ixCol].end());
			paperSide.emplace_back(std::move(str));
		}
		results.emplace_back(paperSide);
	}
	return results;
	
}


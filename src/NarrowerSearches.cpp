#include "NarrowerSearches.h"
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <future>
#include <thread>
#include "utillites.h"

void NarrowerSearches::buildScores() {
	
	auto worker = [this](int dist_from_first, int size) {
		std::array<char,N>  tw;
		Indexes index = indexNexter.getIndexes(dist_from_first);
		for (int i = 0; i < size; i++) {
			auto & score = cols2_scores[i + dist_from_first];
			for (size_t j = 0; j < inputData.size(); j++)
			{
				auto &left = inputData[j][ index[0]];
				auto &mid = inputData[j][index[1]];
				auto &right = inputData[j][index[2]];
				std::copy(left.begin(), left.end(), tw.begin());
				std::copy(mid.begin(), mid.end(), tw.begin() + 2);
				std::copy(right.begin(), right.end(), tw.begin()+4);
				score += rater.getScore(tw);
			}
			indexNexter.initToNext(index);
		}
		return 1;
	};
	using Future = std::future<int>;
	std::list<Future> futures;

	int size = indexNexter.howManyPermutation() / std::thread::hardware_concurrency();
	size_t i = 0;
	for (i; i < std::thread::hardware_concurrency()-1; i++)	{
		futures.push_back(std::async(std::launch::async, worker, (int)i*size, size));
	}
	 worker( (int)i*size,(int)indexNexter.howManyPermutation()  - size*(int)i );

	while (futures.size()) {
		futures.front().get();
		futures.pop_front();
	}

}

std::vector<PaperSide> NarrowerSearches::getBestSugestions(int numbersOfSugestins) {
	using TempPermutation = std::list<int>;
	std::vector<ColumnsPermutation>  permutations;
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
		permutation.push_back(indexes[next][1]);
		permutation.push_back(indexes[next][2]);
		next++;
		while (next != indexes.size()&&permutation.size()<cols) {
			if (permutation.back() == indexes[next][0] &&
				//*(++permutation.rbegin()) == indexes[next][-1]&&
				isNotAddedEarlier(indexes[next][1])&&
				isNotAddedEarlier(indexes[next][2]))
			{
				permutation.push_back(indexes[next][1]);
				permutation.push_back(indexes[next][2]);
			} else 	if (permutation.front() == indexes[next][2] &&
//				*(++permutation.begin()) == indexes[next][1] &&
				isNotAddedEarlier(indexes[next][0])&&
				isNotAddedEarlier(indexes[next][1]))
			{
					permutation.push_front(indexes[next][0]);
					permutation.push_front(indexes[next][1]);
			}
			next++;
		}
		if (permutation.size() == cols) {
			permutations.push_back(ColumnsPermutation(permutation.begin(), permutation.end()));
			start++;
		}
		else
			break;
	}
	return getPapers(permutations);
}

std::vector<PaperSide> NarrowerSearches::getPapers(const std::vector<ColumnsPermutation> & colesPermutatios)const
{
	std::vector<PaperSide> results;
	for (int i = 0; i < colesPermutatios.size(); i++) {
		results.emplace_back(getPageSizeFromColumnPermutation(inputData, colesPermutatios[i]));
	}
	return results;

}
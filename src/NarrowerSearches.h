#pragma once
#include "RatingGivers.h"
#include "GrowingPermutation.h"
#include <fstream>
#include<future>
template <int CharsSize>
class NarrowerSearches
{
	const VectorOrRows & inputData;
	static constexpr int PermutationSize = CharsSize/2 ;
	const CombinedRaingGiver<CharsSize>& rater;
	const size_t rows;
	const size_t cols;
	const InitializerOfIndexsNPermutation<PermutationSize> indexNexter;
	std::vector<unsigned long long >  cols2_scores;
	std::vector<typename InitializerOfIndexsNPermutation<PermutationSize>::Indexes> indexes;
	using Indexes = typename  InitializerOfIndexsNPermutation<PermutationSize>::Indexes;
	void buildScores();

	std::vector<Indexes> getAllIndexes() {
		Indexes index = indexNexter.getFirst();
		std::vector<Indexes> indexes(cols2_scores.size());
		for (size_t i = 0; i < indexes.size(); i++) {
			indexes[i] = index;
			indexNexter.initToNext(index);
		}
		return indexes;
	}

	std::vector<Indexes> getSortedIndexes() {
		std::vector<unsigned int > positions(cols2_scores.size());
		auto indexes=getAllIndexes();
	//	std::ofstream out("C:\\Users\\dell\\Documents\\revert-paper-shredder\\revert-paper-shredder\\Empty\\Empty\\Wyniki.txt");
		std::vector<Indexes> retIndexes( indexes.size());
		//out << cols2_scores.size() << std::endl;

		for( unsigned int i=0; i< positions.size();i++)	positions[i] = i;
		std::sort(positions.begin(), positions.end(), [this](auto & p1, auto &p2) {
			return cols2_scores[p1] > cols2_scores[p2]; });

		for (size_t i = 0; i < positions.size(); i++)	{
			retIndexes[i] = indexes[positions[i]];

	//		out << cols2_scores[i] << " ";

		}
		//std::copy(retIndexes.begin(), retIndexes.begin()+indexes.size(), retIndexes.begin()+indexes.size());
		//std::copy(retIndexes.begin(), retIndexes.begin()+2*indexes.size(), retIndexes.begin()+2*indexes.size());

		return retIndexes;
	}

	std::vector<PaperSide> getPapers(const std::vector<ColumnsPermutation> & colesPermutatios)const ;

public:
	NarrowerSearches(const VectorOrRows & data, const   CombinedRaingGiver<CharsSize> &rater):inputData(data), rater(rater), rows(data.size()), cols(data.front().size()), indexNexter((int)data.front().size())
	{
		cols2_scores = std::vector<unsigned long long >(indexNexter.howManyPermutation(),0ull);
		buildScores();
		indexes = getSortedIndexes();
	}
	template<int OverlapingSize>
	std::vector<PaperSide> getBestSugestions(int nuumbersOfSugestins);
};



template <int CharsSize>
void NarrowerSearches<CharsSize>::buildScores() {

	auto worker = [this](int dist_from_first, int size) {
		std::array<char, CharsSize>  tw;
		Indexes index = indexNexter.getIndexes(dist_from_first);
		for (int i = 0; i < size; i++) {
			auto & score = cols2_scores[i + dist_from_first];
			for (size_t j = 0; j < inputData.size(); j++)
			{
				for (int ix = 0, off = 0;
					ix < PermutationSize;
					ix++, off += 2) {
					auto &twoChars = inputData[j][index[ix]];
					std::copy(twoChars.begin(), twoChars.end(), tw.begin() + off);
				}
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
	for (i; i < std::thread::hardware_concurrency() - 1; i++) {
		futures.push_back(std::async(std::launch::async, worker, (int)i*size, size));
	}
	worker((int)i*size, (int)indexNexter.howManyPermutation() - size*(int)i);

	while (futures.size()) {
		futures.front().get();
		futures.pop_front();
	}

}


template <int CharsSize>
template< int OverlapSize>
std::vector<PaperSide> NarrowerSearches<CharsSize>::getBestSugestions(int numbersOfSugestins) {
	std::vector<ColumnsPermutation>  permutations;
	int i = 0 ;
	numbersOfSugestins = numbersOfSugestins > (int)indexes.size() / 3 ? (int)indexes.size() / 3 : numbersOfSugestins;
	GrowingPermutation<PermutationSize, OverlapSize> growing_permutation;
	for (; i < numbersOfSugestins; i++) {
		int next = 0;
		growing_permutation.init(indexes[i]);
		while (next < indexes.size() && growing_permutation.size() < cols) {
			if (growing_permutation.tryAdd(indexes[next]))
				next = 0;
			else
				next++;
		}
		if (growing_permutation.size() == cols) {
			permutations.push_back(growing_permutation.getVecCols());
		}
	}
	return getPapers(permutations);
}

template <int CharsSize>
std::vector<PaperSide> NarrowerSearches<CharsSize>::getPapers(const std::vector<ColumnsPermutation> & colesPermutatios)const
{
	std::vector<PaperSide> results;
	for (int i = 0; i < colesPermutatios.size(); i++) {
		results.emplace_back(getPageSizeFromColumnPermutation(inputData, colesPermutatios[i]));
	}
	return results;

}


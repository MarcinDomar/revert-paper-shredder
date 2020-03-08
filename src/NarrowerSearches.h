#pragma once
#include "RatingGivers.h"
#include "GrowingPermutation.h"
#include <fstream>
#include<future>
#include<tuple>


template <int CharsSize, typename ColIndexType = unsigned char>
class NarrowerSearches
{
	static constexpr int PermutationSize = CharsSize / 2;
	using ColumnsPermutation = std::vector<ColIndexType>;
	using Indexes = typename  InitializerOfIndexsNPermutation<PermutationSize, ColIndexType>::Indexes;
	const VectorOfRows & inputData;
	const size_t rows;
	const size_t cols;
	const InitializerOfIndexsNPermutation<PermutationSize, ColIndexType> indexNexter;


	std::vector<Indexes> vecIndexes;
	std::vector<Indexes> vecBadIndexes;
	template <int RatierCharsSize>
	void findSortedVecOfIndexes(const   CombinedRatingGiver<RatierCharsSize> &rater);

	std::vector<Indexes> getAllIndexes() {
		Indexes index = indexNexter.getFirst();
		std::vector<Indexes> vecIndexes(indexNexter.getSizeOfAllPermutations());
		for (size_t i = 0; i < vecIndexes.size(); i++) {
			vecIndexes[i] = index;
			indexNexter.initToNext(index);
		}
		return vecIndexes;
	}

	void initVectorsIndexes(const std::vector<short> &cols_scores);

	std::vector<PaperSide> getPapers(const std::vector<ColumnsPermutation> & colesPermutatios)const;

public:
	NarrowerSearches(const VectorOfRows & data) :inputData(data), rows(data.size()), cols(data.front().size()), indexNexter((int)data.front().size())
	{
	}
	template<int OverlapingSize, int RatierCharsSize>
	std::vector<PaperSide> getBestSugestions(int nuumbersOfSugestins, const   CombinedRatingGiver<RatierCharsSize> &rater);
};


template <int CharsSize, typename RatingGiver>
struct ScoreCounter {
	std::array<char, CharsSize> chars;
	const RatingGiver & ratingGiver;
	const VectorOfRows &rows;
public:
	ScoreCounter(const RatingGiver & ratingGiver, const VectorOfRows & rows) : rows(rows), ratingGiver(ratingGiver) {}

	template <typename ColIndexType>
	auto  countScore(const std::array<ColIndexType, CharsSize/2> & indexes) {
		decltype(ratingGiver.getScore(chars)) score = 0;
		for (auto & r : rows) {
			size_t off = 0;
			for (auto ix : indexes) {
				auto & twoChars = r[ix];
				std::copy(twoChars.begin(), twoChars.end(), chars.begin() + off);
				off += 2;
			}
			score += ratingGiver.getScore(chars);
		}
		return score;
	}
};


template <int PermutationSize, int OverlappingSize, typename ColIndexesType>
struct TwoJoindedIndexes :public std::array<ColIndexesType, 2 * PermutationSize - OverlappingSize> {
	using Indexes = std::array<ColIndexesType, PermutationSize>;
	const std::array<ColIndexesType, 2 * PermutationSize - OverlappingSize> & getConstRef()const { return *this; }
	void intLeft(const Indexes& indexes) {
		std::copy(indexes.begin(), indexes.end(), begin());
	}

	void intRight(const Indexes& indexes) {
		std::copy(indexes.begin(), indexes.end(), end() - PermutationSize);
	}

	void setLeft(const Indexes& indexes) {
		std::copy(indexes.begin(), indexes.end() - OverlappingSize, begin());
	}

	void setRight(const Indexes& indexes) {
		std::copy(indexes.begin(), indexes.end() - OverlappingSize, begin() + PermutationSize);
	}
};


template <typename Indexes>
class SuggestionServiceCtlr {

	const int  limit;
	int counter = 0;
	Indexes lastIndexes;
	short lastScore;

public:
	SuggestionServiceCtlr(int limit) :limit(limit) {};

	bool isHearCandidate(const Indexes& indexes, short score) {
		if (counter == 0) {
			lastScore = score;
			lastIndexes = indexes;
		}
		else if (score > lastScore)
		{
			lastScore = score;
			lastIndexes = indexes;
		}
		counter++;
		if (counter == limit)
		{
			counter = 0;
			return true;
		}
		return false;
	}

	const Indexes &getIndexes()const {
		return indexes;
	}

	void reset() {
		counter = 0;
	}
	bool isEmpty()const { return counter == 0; }
	static int  getBetterIndexes(SuggestionServiceCtlr<Indexes> & leftServ, SuggestionServiceCtlr<Indexes> & rightServ, Indexes & retIndexes) {
		if (leftServ.counter > 0 && rightServ.counter > 0) {
			if (leftServ.lastScore < rightServ.lastScore) {
				retIndexes =rightServ.lastIndexes;
				rightServ.counter = 0;
				return 1;
			}
			retIndexes = leftServ.lastIndexes;
			leftServ.counter = 0;
			return -1;
		}
		else if (leftServ.counter > 0) {
			leftServ.counter = 0;
			retIndexes = leftServ.lastIndexes;
			return -1;
		}
		else if (rightServ.counter > 0) {
			rightServ.counter = 0;
			retIndexes = rightServ.lastIndexes;
			return 1;
		}
		else
			return 0;

	}

};



template <int CharsSize, typename ColIndexType>
void NarrowerSearches<CharsSize, ColIndexType>::initVectorsIndexes(const std::vector<short> &cols_scores) {
	std::vector<unsigned int > positions(cols_scores.size());
	auto retIndexes= getAllIndexes();
	vecIndexes.assign(retIndexes.size(), {});
	auto cols_predicate = [&cols_scores](auto & p1, auto & p2) {
		return cols_scores[p1] > cols_scores[p2];
	};

	for (unsigned int i = 0; i < positions.size(); i++)	positions[i] = i;
	std::sort(positions.begin(), positions.end(), cols_predicate);

	for (size_t i = 0; i < positions.size(); i++) {
		vecIndexes[i] = retIndexes[positions[i]];
	}

	auto size_of_permutations_considarated = std::lower_bound(positions.begin(), positions.end(), 0, cols_predicate) - positions.begin();
	vecBadIndexes = std::vector<Indexes>(vecIndexes.begin() + size_of_permutations_considarated, vecIndexes.end());
	vecIndexes.erase(vecIndexes.begin() + size_of_permutations_considarated, vecIndexes.end());
}


template <int CharsSize, typename ColIndexType>
template <int RatierCharsSize>
void NarrowerSearches<CharsSize, ColIndexType>::findSortedVecOfIndexes(const   CombinedRatingGiver<RatierCharsSize> &ratier) {

	auto cols_scores = std::vector< short >(indexNexter.getSizeOfAllPermutations(), 0ll);
	auto worker = [this, &cols_scores, &ratier](int dist_from_first, int size) {
		ScoreCounter<CharsSize, CombinedRatingGiver<RatierCharsSize> > scoreCounter(ratier, inputData);

		Indexes index = indexNexter.getIndexes(dist_from_first);
		for (int i = 0; i < size; i++) {
			cols_scores[i + dist_from_first] = scoreCounter.countScore(index);
			indexNexter.initToNext(index);
		}
		return 1;
	};
	using Future = std::future<int>;
	std::list<Future> futures;

	int size = indexNexter.getSizeOfAllPermutations() / std::thread::hardware_concurrency();
	size_t i = 0;
	for (i; i < std::thread::hardware_concurrency() - 1; i++) {
		futures.push_back(std::async(std::launch::async, worker, (int)i*size, size));
	}
	worker((int)i*size, (int)indexNexter.getSizeOfAllPermutations() - size*(int)i);

	while (futures.size()) {
		futures.front().get();
		futures.pop_front();
	}
	 initVectorsIndexes(cols_scores);

}

template <int CharsSize, typename ColIndexType>
template< int OverlappingSize, int RatierCharsSize>
std::vector<PaperSide> NarrowerSearches<CharsSize, ColIndexType>::getBestSugestions(int numbersOfSugestins, const   CombinedRatingGiver<RatierCharsSize> &ratier) {

	findSortedVecOfIndexes(ratier);
	std::vector<ColumnsPermutation>  permutations;
	int i = 0;
	numbersOfSugestins = numbersOfSugestins > (int)vecIndexes.size() ? (int)vecIndexes.size() : numbersOfSugestins;
	GrowingPermutation<PermutationSize, OverlappingSize, ColIndexType> growing_permutation;
	int cols_ = ((int)cols - PermutationSize) / (PermutationSize - OverlappingSize)*(PermutationSize - OverlappingSize) + PermutationSize;
	for (; i < numbersOfSugestins; i++) {
		int next = 0;
		growing_permutation.init(vecIndexes[i]);
		while (next < vecIndexes.size() && growing_permutation.size() < cols_) {
			if (growing_permutation.tryAdd(vecIndexes[next]))
				next = 0;
			else
				next++;
		}
		if (cols != cols_) {
			while (next < vecIndexes.size() && growing_permutation.size() < cols) {
				if (growing_permutation.tryAdd(vecIndexes[next], PermutationSize-((int)cols - cols_)))
					next = 0;
				else
					next++;
			}
		}
		if (growing_permutation.size() == cols) {
			permutations.push_back(growing_permutation.getVecCols());
		}
	}
	return getPapers(permutations);
}

template <int CharsSize, typename ColIndexType>
std::vector<PaperSide> NarrowerSearches<CharsSize, ColIndexType>::getPapers(const std::vector<ColumnsPermutation> & colesPermutatios)const
{
	std::vector<PaperSide> results;
	for (int i = 0; i < colesPermutatios.size(); i++) {
		results.emplace_back(getPageSizeFromColumnPermutation(inputData, colesPermutatios[i]));
	}
	return results;

}


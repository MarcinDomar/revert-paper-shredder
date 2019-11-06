#pragma once
#include "Types.h"
#include "RatingGivers.h"

class NarrowerSearches
{


	const VectorOrRows & inputData;
	const CombinedRaingGiver<6>& rater;
	const size_t rows;
	const size_t cols;
	const InitializerOfIndex3Permutaion indexNexter;
	std::vector<unsigned long long >  cols2_scores;
	std::vector<Index3Perm> indexes;

	void buildScores();
	
	std::vector<Index3Perm> getAllIndexes() {
		Index3Perm index = indexNexter.getFirst();
		std::vector<Index3Perm> indexes(cols2_scores.size());
		for (size_t i = 0; i < indexes.size(); i++) {
			indexes[i] = index;
			indexNexter.initToNext(index);
		}
		return indexes;
	}

	std::vector<Index3Perm> getSortedIndexes() {
		std::vector<unsigned int > positions(cols2_scores.size());
		auto indexes=getAllIndexes();
		std::vector<Index3Perm> retIndexes(8 * indexes.size());

		for( unsigned int i=0; i< positions.size();i++)	positions[i] = i;
		std::sort(positions.begin(), positions.end(), [this](auto & p1, auto &p2) {
			return cols2_scores[p1] > cols2_scores[p2]; });
		for (int i = 0; i < positions.size(); i++)	{
			retIndexes[i] = indexes[positions[i]];
		}
		std::copy(indexes.begin(), indexes.end(), retIndexes.begin());
		std::copy(retIndexes.begin(), retIndexes.begin()+indexes.size(), retIndexes.begin()+indexes.size());
		return retIndexes;
	}
	
	std::vector<PaperSide> getPapers(const std::vector<ColumnsPermutation> & colesPermutatios)const ;

public:
	NarrowerSearches(const VectorOrRows & data, const   CombinedRaingGiver<6> &rater):inputData(data), rater(rater), rows(data.size()), cols(data.front().size()), indexNexter(data.front().size())
	{
		cols2_scores = std::vector<unsigned long long >(indexNexter.howManyPermutaion(),0ull);
		buildScores();
		indexes = getSortedIndexes();
	}
	std::vector<PaperSide> getBestSugestions(int nuumbersOfSugestins);
};


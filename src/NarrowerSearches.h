#pragma once
#include "RatingGivers.h"
#include <fstream>
class NarrowerSearches
{
	static constexpr int N = 8;
	using Indexes = InitializerOfIndexsNPermutation<N/2>::Indexes;
	const VectorOrRows & inputData;
	const CombinedRaingGiver<N>& rater;
	const size_t rows;
	const size_t cols;
	const InitializerOfIndexsNPermutation<N/2> indexNexter;
	std::vector<unsigned long long >  cols2_scores;
	std::vector<Indexes> indexes;

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
		std::vector<Indexes> retIndexes(2 * indexes.size());


		for( unsigned int i=0; i< positions.size();i++)	positions[i] = i;
		std::sort(positions.begin(), positions.end(), [this](auto & p1, auto &p2) {
			return cols2_scores[p1] > cols2_scores[p2]; });

		for (size_t i = 0; i < positions.size(); i++)	{
			retIndexes[i] = indexes[positions[i]];
		//out << cols2_scores[i] << " ";
		}
		std::copy(indexes.begin(), indexes.end(), retIndexes.begin());
		std::copy(retIndexes.begin(), retIndexes.begin()+indexes.size(), retIndexes.begin()+indexes.size());
		return retIndexes;
	}
	
	std::vector<PaperSide> getPapers(const std::vector<ColumnsPermutation> & colesPermutatios)const ;

public:
	NarrowerSearches(const VectorOrRows & data, const   CombinedRaingGiver<N> &rater):inputData(data), rater(rater), rows(data.size()), cols(data.front().size()), indexNexter((int)data.front().size())
	{
		cols2_scores = std::vector<unsigned long long >(indexNexter.howManyPermutation(),0ull);
		buildScores();
		indexes = getSortedIndexes();
	}
	std::vector<PaperSide> getBestSugestions(int nuumbersOfSugestins);
};


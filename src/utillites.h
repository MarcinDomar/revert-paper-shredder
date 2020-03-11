#pragma once
#include "types.h"
#include "InitializerOfIndexes.h"

ListOfRows  readListOfRows(const std::string filename);

VectorOfRows getVectorOfRows(const std::string filename);


PaperSide getPaperSizeFromFile(const std::string & fileName);

template  <typename ColIndexType>
auto getPermutationFromOriginPage(const PaperSide & originText, const VectorOfRows & scrumbled) {
	std::vector<ColIndexType> positionInOriginText(scrumbled.front().size());

	for (int i = 0; i < positionInOriginText.size(); i++) {
		for (int j = 0, k; j < positionInOriginText.size(); j++) {
			if (positionInOriginText.begin() + i == std::find(positionInOriginText.begin(), positionInOriginText.begin() + i, j))
			{
				for (k = 0; k < scrumbled.size(); k++) {
					auto & rowO = originText[k];
					auto & rowS = scrumbled[k];
					if (rowO[2 * j] != rowS[i][0] || rowO[2 * j + 1] != rowS[i][1])
						break;
				}
				if (k == scrumbled.size()) {
					positionInOriginText[i] = j;
					break;
				}
			}
		}
	}
	std::vector<ColIndexType> result(positionInOriginText.size());
	for (int i = 0; i < result.size(); i++) {
		result[positionInOriginText[i]] = i;
	}
	return result;
}
template <typename ColIndexType>
unsigned int getRatingOfPermutation(const std::vector<ColIndexType> & origin, const std::vector<ColIndexType> & toScore)
{
	unsigned int score = 0;
	for (int i = 0, j; i < toScore.size(); )
	{
		for (j = 0; j < toScore.size(); j++)
		{
			if (origin[i] == toScore[j])
				break;
		}
		unsigned int rate = 0;
		i++;
		for (; j < toScore.size() && i < toScore.size() && origin[i] == toScore[j]; j++, i++)
			rate++;
		score += rate*rate;
	}
	return score;
}



template <typename ColIndexType>
PaperSide getPageSideFromColumnPermutation(const VectorOfRows & scrumbled, const std::vector<ColIndexType> & columnsPermutations) {
	auto rows = scrumbled.size();
	PaperSide paperSide;
	for (int row = 0; row != rows; row++) {
		std::string str;
		for (auto ixCol : columnsPermutations)
			str.insert(str.end(), scrumbled[row][ixCol].begin(), scrumbled[row][ixCol].end());
		paperSide.emplace_back(std::move(str));
	}
	return paperSide;
}

template < typename ColIndexType>
std::vector<PaperSide> getPapers(VectorOfRows & inputStripes, const std::vector<std::vector<ColIndexType>> & colesPermutations)
{
	std::vector<PaperSide> results;
	for (int i = 0; i < colesPermutations.size(); i++) {
		results.emplace_back(getPageSideFromColumnPermutation(inputStripes, colesPermutations[i]));
	}
	return results;

}
template < typename PageRatier>
class PermutationRatier {
public:
	struct ScoreIx {
		unsigned int ix;
		int score;

		bool operator <(const ScoreIx & si)const { return score < si.score; }
	};
private:
	const VectorOfRows & inputStripes;
	const PageRatier & pageRatier;
public:
	PermutationRatier(const VectorOfRows & inputStripes, const PageRatier & pageRatierGiver) :inputStripes(inputStripes), pageRatier(pageRatierGiver) {}
	template <typename ColIndexType>
	std::vector<ScoreIx> operator()(const std::vector<std::vector<ColIndexType>> & vecPermutations)const;
};

template <typename PageRatier>
template<typename  ColIndexType>
std::vector<typename PermutationRatier< PageRatier>::ScoreIx> PermutationRatier< PageRatier>:: operator()(const std::vector<std::vector<ColIndexType>> & vecPermutations)const {
	std::vector<ScoreIx> result;
	result.reserve(vecPermutations.size());
	for (size_t i = 0; i < vecPermutations.size(); i++) {
		result.push_back({ (unsigned int)i,pageRatier.getScore(getPageSideFromColumnPermutation(inputStripes, vecPermutations[i])) });
	}
	std::sort(result.begin(), result.end());
	return result;
}

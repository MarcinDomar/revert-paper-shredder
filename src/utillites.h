#pragma once
/*
Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2020 Marcin Domarski.
Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "types.h"
#include "InitializerOfIndexes.h"

ListOfRows  readListOfRows(const std::string &filename);

VectorOfRows getVectorOfRows(const std::string &filename);

VectorOfRows getLowerVectorOfRows(const VectorOfRows& vecOrRows);


PaperSide getPaperSideFromFile(const std::string & fileName);

template  <typename ColIndexType>
auto getPermutationFromOriginPage(const PaperSide & originText_, const VectorOfRows & scrumbled) {
	auto originText = originText_;
	std::vector<ColIndexType> positionInOriginText(scrumbled.front().size());
	auto fixedSize = scrumbled.front().size() * 2;
	for (auto &row : originText) {
		while (row.size() < fixedSize) {
			row.append(" "	);
		}
	}

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
	std::vector<ScoreIx> result(vecPermutations.size());
	auto job=[&result,this,&vecPermutations](size_t beg, size_t size) {
		for (size_t i = beg, iend = beg + size; i < iend; i++) {
			result[i].ix = (unsigned int)i;
			result[i].score = pageRatier.getScore(getPageSideFromColumnPermutation(inputStripes, vecPermutations[i]));
		}
		return 0;
	};
	std::list<std::future<int>> futures;
	result.reserve(vecPermutations.size());
	size_t i = 0, delta = vecPermutations.size() / std::thread::hardware_concurrency();

	for (; i < std::thread::hardware_concurrency() - 1; i++)
		futures.push_back(std::async(std::launch::async, job, i*delta, delta));
	job(i*delta , result.size() - i*delta);

	while (futures.size()) {
		futures.front().get();
		futures.pop_front();
	}


	std::sort(result.begin(), result.end());
	return result;
}

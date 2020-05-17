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
#include <iostream>
#include <stdexcept>
#include "utillites.h"
#include "RatingGivers.h"
#include "SmallPermutationNarrower.h"
#include "SelectorColumnPermutations.h"
#include <chrono>
#include <iomanip>

using namespace std;



template <typename ColIndexType,int CharsSize>
void show_algorithm_results_for_parameters(const char *dictionary_file_name,const VectorOfRows & inputStripes,const std::vector<ColIndexType> & correctPermutation)
{
	auto t1 = std::chrono::steady_clock::now();
	auto getMilliseconds = [&t1] {
		auto t2 = std::chrono::steady_clock::now();
		auto d = t2 - t1;
		t1 = t2;
		return std::chrono::duration_cast<std::chrono::milliseconds>(d).count();
	};
	static_assert(CharsSize % 2 == 0, "CharsSize should be multipication of 2 ");
	getMilliseconds();
	std::cout << "An attempt  of find the correct order of columns with " <<  CharsSize / 2 << " size of Permutation\n";
	auto ratingGiver = getRatingGiver<CombinedRatingGiver<CharsSize>>(dictionary_file_name );
	std::cout << "I've got RatingGiver for sibbling  letters. It took " << getMilliseconds() << " millisecons\n" << std::endl;


	SmallPermutationNarrower<CharsSize, CombinedRatingGiver<CharsSize>, ColIndexType> spermutationNarrower(inputStripes, ratingGiver);
	auto vecIndexes = spermutationNarrower();
	std::cout << "Narrow " << (CharsSize / 2) << " from " << inputStripes.front().size() << " permutations , finded " << vecIndexes.size() << " intersting permutations.\n It took " << getMilliseconds() << " millisecons\n" << std::endl;
	SelectorOfBestPosssibleColumnPermutations<CharsSize / 2, 0/*OverlappingSize*/, ColIndexType> narrowerSelector0(vecIndexes, (int)inputStripes.front().size());
	auto suggestedPages0 = narrowerSelector0();
	auto dur0 = getMilliseconds();

	SelectorOfBestPosssibleColumnPermutations<CharsSize / 2, 1/*OverlappingSize*/, ColIndexType> narrowerSelector1(vecIndexes, (int)inputStripes.front().size());
	auto suggestedPages1 = narrowerSelector1();
	auto dur1 = getMilliseconds();

	SelectorOfBestPosssibleColumnPermutations<CharsSize / 2, 2/*OverlappingSize*/, ColIndexType> narrowerSelector2(vecIndexes, (int)inputStripes.front().size());
	auto suggestedPages2 = narrowerSelector1();
	auto dur2 = getMilliseconds();

	PermutationRatier< CombinedRatingGiver<CharsSize>> permutationRatier(inputStripes, ratingGiver);
	auto showResult = [&](auto &suggestedPages, auto duration ,int overlapping) {

		std::cout << "Overlapping : " <<overlapping <<" Suggestions : "  << suggestedPages1.size() << "   pages . It took  " << duration << " milliseconds" << std::endl;
		auto vecScoreIx = permutationRatier(suggestedPages);
		std::vector < std::string> numerals{ "First","Second", "Third", "Fourth" };
		if (vecScoreIx.size()) {
			auto it = numerals.rbegin();
			for (size_t i = vecScoreIx.size() > 4 ? vecScoreIx.size() - 4 : 0; i < vecScoreIx.size(); i++) {
				bool Ok = std::equal(correctPermutation.begin(),correctPermutation.end(), suggestedPages[vecScoreIx[i].ix].begin());
				std::cout << std::setw(7) << (*it++) << " place score: " << std::setw(6) << vecScoreIx[i].score << ", position in set " << std::setw(6) << vecScoreIx[i].ix << (Ok ? " Correct Permutatilon" : "") << "\n";
			}
			std::cout << "--------------------------\n";
		}
		else {
			std::cerr << "Nothing to return " << std::endl;
		}
	};
	showResult(suggestedPages0, dur0,0);
	showResult(suggestedPages1, dur1,1);
	showResult(suggestedPages2, dur1,2);
}

int main(int argc, char *argv[])
{
	using ColIndex = unsigned char;
	if (argc != 4) {
		std::cerr << "As first parameter pass path to file with shredded text" << std::endl;
		std::exit(-1);
	}

	auto inputStripesO = getVectorOfRows(argv[1]);
	auto inputStripes = getLowerVectorOfRows(inputStripesO);
	auto correctPermutation = getPermutationFromOriginPage<ColIndex>(getPaperSideFromFile(argv[2]), inputStripesO);
	show_algorithm_results_for_parameters<ColIndex,6>(argv[3], inputStripes, correctPermutation);
	show_algorithm_results_for_parameters<ColIndex,8>(argv[3], inputStripes, correctPermutation);
	show_algorithm_results_for_parameters<ColIndex,10>(argv[3], inputStripes, correctPermutation);
	int i;
	std::cin >> i;
	return 0;
}
